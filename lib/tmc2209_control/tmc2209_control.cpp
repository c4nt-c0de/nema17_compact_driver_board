#include "tmc2209_control.h"

void tmc2209_setup(
    TMC2209Stepper *driver,
    uint8_t dir_pin,
    uint8_t step_pin,
    uint8_t stdby_pin,
    uint8_t enn_pin,
    uint8_t rx_pin,
    uint8_t tx_pin,
    uint8_t diag_pin
) {
    if (!driver) {
        Serial.println("Driver is NULL");
        return;
    };

    pinMode(dir_pin, OUTPUT);
    pinMode(step_pin, OUTPUT);
    pinMode(stdby_pin, OUTPUT);
    pinMode(enn_pin, OUTPUT);
    pinMode(diag_pin, INPUT_PULLUP); // can't use it with the setup I got

    digitalWrite(stdby_pin, LOW);
    digitalWrite(enn_pin, HIGH);
    digitalWrite(dir_pin, HIGH);

    Serial1.begin(115200, SERIAL_8N1, rx_pin, tx_pin);

    driver->begin();
    driver->en_spreadCycle(true);
    driver->rms_current(200);
    driver->microsteps(4);
}


void tmc2209_enable(int enn_pin) {
    digitalWrite(enn_pin, LOW);
}

void tmc2209_disable(int enn_pin) {
    digitalWrite(enn_pin, HIGH);
}

void do_step(uint8_t step_pin, uint8_t dir_pin, uint8_t enn_pin, bool dir_high) {
    digitalWrite(dir_pin, dir_high ? HIGH : LOW);
    digitalWrite(enn_pin, LOW);
    delayMicroseconds(500);

    digitalWrite(step_pin, HIGH);
    delayMicroseconds(20);
    digitalWrite(step_pin, LOW);
    delayMicroseconds(20);

    digitalWrite(enn_pin, HIGH);
}

// Spin motor by a signed angle (deg)
void spin_motor(uint8_t step_pin, uint8_t dir_pin, uint8_t enn_pin, float angle_deg) {
    if (angle_deg == 0) return;  // nothing to do

    pinMode(step_pin, OUTPUT);
    pinMode(dir_pin, OUTPUT);
    pinMode(enn_pin, OUTPUT);

    bool dir_high = (angle_deg > 0);  // positive -> counterclockwise
    int steps = abs(int(angle_deg));  // convert to number of steps

    for (int i = 0; i < steps; i++) {
        do_step(step_pin, dir_pin, enn_pin, dir_high);
    }
}