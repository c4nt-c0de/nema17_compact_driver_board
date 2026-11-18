#include <TMCStepper.h>
#include "OTA_handler.h"

#include "webpage.h"
#include "tmc2209_control.h"
#include "AS5600_encoder.h"
#include "ir_receiver.h"


#define ESP_WEBPAGE_HOSTNAME "myesp"
#define SSID "XXX"
#define PASSWORD "YYY"

// pin definitions
#define BOOT_PIN       0
#define DIR_PIN        1
#define STEP_PIN       2
#define INDEX_PIN      3
#define DIAG_PIN       4
#define ENN_PIN        5
#define STDBY_PIN      6
#define TX_PIN         17
#define RX_PIN         18


#define TOUCH1_PIN     7
#define TOUCH2_PIN     8

#define XTAL1_PIN      15
#define XTAL2_PIN      16

#define USB_DN_PIN     19
#define USB_DP_PIN     20

#define IR_PIN         21
#define ENCODER_PIN    TOUCH2_PIN
#define DEBUG_LED_PIN  42

#define TMC2209_ADDRESS 0
#define R_SENSE 0.1f

OTA_handler ota;
TMC2209Stepper driver(&Serial1, R_SENSE, TMC2209_ADDRESS);

bool ir_triggered = false;
bool stall_guard_triggered = false;
volatile bool ir_event_flag = false;
volatile float shaft_angle_deg = 0.0f;
volatile float total_angle = 0;
float last_angle_deg = 0.0f;
int rotations = 0; // accumulated rotations

void toggle_led(int pin_num) {
    digitalWrite(pin_num, !digitalRead(pin_num));
}

float get_esp_temp() {
  return temperatureRead();
}

uint16_t read_stallguard()
{
    return driver.SG_RESULT();
}

void IR_ISR() {
    ir_event_flag = true;
}


void handle_boot_button() { // only one I managed to get working
    pinMode(BOOT_PIN, INPUT);

    if (digitalRead(BOOT_PIN) == LOW) { 
        digitalWrite(ENN_PIN, LOW);
        delayMicroseconds(500);
        digitalWrite(DEBUG_LED_PIN, HIGH);
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(10);
        digitalWrite(ENN_PIN, HIGH);
    } else {
        digitalWrite(DEBUG_LED_PIN, LOW);
    }
}


void setup() {

    tmc2209_setup(
        &driver,
        DIR_PIN,
        STEP_PIN, 
        STDBY_PIN,
        ENN_PIN,
        RX_PIN,
        TX_PIN,
        DIAG_PIN
    );

    pinMode(IR_PIN, INPUT);
    pinMode(DEBUG_LED_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(IR_PIN), IR_ISR, RISING);

    ota.begin(SSID, PASSWORD, [](void* server){
        register_web_routes(
            server,
            [](){ toggle_led(DEBUG_LED_PIN); },
            [](){ return get_esp_temp(); },
            [](){ return ir_triggered; },
            [](){ return read_stallguard(); },
            &shaft_angle_deg,
            &total_angle
        );
    }, ESP_WEBPAGE_HOSTNAME);
}

void loop() {
    ota.handle();
    if(ir_event_flag) {
        ir_triggered = true;
        ir_event_flag = false;
        spin_motor(STEP_PIN, DIR_PIN, ENN_PIN, -total_angle);
        shaft_angle_deg = read_shaft_angle_deg(ENCODER_PIN);
        last_angle_deg = shaft_angle_deg;
        total_angle = shaft_angle_deg;
        rotations = 0;
    } else {
        ir_triggered = false;
    }
    
    handle_boot_button();
    shaft_angle_deg = read_shaft_angle_deg(ENCODER_PIN);
    float delta = shaft_angle_deg - last_angle_deg;
    if(delta > 180.0f) {
        rotations -= 1;
    } else if(delta < -180.0f) {
        rotations += 1;
    }
    last_angle_deg = shaft_angle_deg;    
    total_angle = shaft_angle_deg + rotations*360.0f;
}


/*
// the non blocking version of the current one
// annoyingly, it doesnt work

void handle_boot_button() {
    pinMode(BOOT_PIN, INPUT);

    if (digitalRead(BOOT_PIN) == LOW) { 
        if (!boot_button_active) {
            // first time pressed
            boot_button_active = true;
            digitalWrite(ENN_PIN, LOW);
            digitalWrite(DEBUG_LED_PIN, HIGH);
            step_last_time = micros();
            step_pin_state = false;
        }

        unsigned long now = micros();
        if (!step_pin_state && now - step_last_time >= 20) {
            digitalWrite(STEP_PIN, HIGH);
            step_last_time = now;
            step_pin_state = true;
        } else if (step_pin_state && now - step_last_time >= 20) {
            digitalWrite(STEP_PIN, LOW);
            step_last_time = now;
            step_pin_state = false;

            // pulse done, disable ENN until next press
            digitalWrite(ENN_PIN, HIGH);
        }

*/