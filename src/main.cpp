#include <TMCStepper.h>
#include "OTA_handler.h"
#include "webpage.h"

#define ESP_WEBPAGE_HOSTNAME "myesp"
#define SSID "Zyxel_6C61"
#define PASSWORD "38UP8TY8HAYDQ8Y3"

// pin definitions
#define BOOT_PIN       0
#define DIR_PIN        1
#define STEP_PIN       2
#define INDEX_PIN      3
#define DIAG_PIN       4
#define ENN_PIN        5
#define STDBY_PIN      6
#define TMC_TX_PIN     17
#define TMC_RX_PIN     18


#define TOUCH1_PIN     7
#define TOUCH2_PIN     8

#define XTAL1_PIN      15
#define XTAL2_PIN      16

#define USB_DN_PIN     19
#define USB_DP_PIN     20

#define IR_PIN         21
#define ENCODER_PIN    39
#define DEBUG_LED_PIN  42

#define TMC2209_ADDRESS 0
#define R_SENSE 0.1f

OTA_handler ota;
TMC2209Stepper driver(&Serial1, R_SENSE, TMC2209_ADDRESS);

bool ir_triggered = false;
bool stall_guard_triggered = false;

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

auto move_motor = []() {
    driver.VACTUAL(1000);  // velocity is set here
    delay(1000);
    driver.VACTUAL(0);
};

const unsigned long stepInterval = 500;
const unsigned long pulseWidth = 50;
unsigned long lastStepTime = 0; // time of last pulse start
bool stepState = false;

void manualStepMotor() {
    unsigned long now = micros();

    if (!stepState && (now - lastStepTime >= stepInterval)) {
        digitalWrite(STEP_PIN, HIGH);
        stepState = true;
        lastStepTime = now; // start counting for pulse width
    }

    if (stepState && (now - lastStepTime >= pulseWidth)) {
        digitalWrite(STEP_PIN, LOW);
        stepState = false;
        lastStepTime = now; // now ready for next stepInterval
    }
}

float get_angle() {
    return random(0, 36000) / 100.0;
}

uint8_t tmc_version = 0;

// Flags for ISR
volatile bool ir_event_flag = false;
volatile bool stall_event_flag = false;

// ISR functions
void IR_ISR() {
    ir_event_flag = true;
}

void STALL_ISR() {
    stall_event_flag = true;
}

void handle_boot_button() {
    pinMode(BOOT_PIN, INPUT);

    if (digitalRead(BOOT_PIN) == LOW) { 
        digitalWrite(ENN_PIN, LOW);
        delayMicroseconds(500);
        //driver.rms_current(200);
        digitalWrite(DEBUG_LED_PIN, HIGH);
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(10);

    } else {
        digitalWrite(DEBUG_LED_PIN, LOW);
    }
}

void setup() {
    Serial.println("Alive and kicking");
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(STDBY_PIN, OUTPUT);
    pinMode(ENN_PIN, OUTPUT);
    digitalWrite(STDBY_PIN, LOW);
    digitalWrite(ENN_PIN, LOW);
    Serial1.begin(115200, SERIAL_8N1, TMC_RX_PIN, TMC_TX_PIN);
    driver.begin();
    driver.en_spreadCycle(true);
    //driver.rms_current(200);
    driver.microsteps(4);

    pinMode(IR_PIN, INPUT);
    pinMode(DIAG_PIN, INPUT_PULLUP); // DIAG usually needs pullup
    pinMode(DEBUG_LED_PIN, OUTPUT);

    tmc_version = driver.version();

    attachInterrupt(digitalPinToInterrupt(IR_PIN), IR_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(DIAG_PIN), STALL_ISR, FALLING);

    ota.begin(SSID, PASSWORD, [](void* server){
        register_web_routes(
            server,
            [](){ toggle_led(DEBUG_LED_PIN); },
            [](){ return get_esp_temp(); },
            [](){ return ir_triggered; },
            [](){ return stall_guard_triggered; },
            [](){ return driver.version(); },
            [](){ return get_angle(); },
            [](){ return read_stallguard(); },
            move_motor
        );
    }, ESP_WEBPAGE_HOSTNAME);
}

void loop() {
    //ota.handle();
/*
    if(ir_event_flag) {
        ir_triggered = true;
        ir_event_flag = false;
        digitalWrite(DEBUG_LED_PIN, HIGH);
        delay(50);
        digitalWrite(DEBUG_LED_PIN, LOW);
    } else {
        ir_triggered = false;
    }

    if(stall_event_flag) {
        stall_guard_triggered = true;
        stall_event_flag = false;
        for(int i=0;i<2;i++){
            digitalWrite(DEBUG_LED_PIN,HIGH);
            delay(50);
            digitalWrite(DEBUG_LED_PIN,LOW);
            delay(50);
        }
    } else {
        stall_guard_triggered = false;
    }
*/
    //manualStepMotor();
    digitalWrite(ENN_PIN, HIGH);
    handle_boot_button();
    

}