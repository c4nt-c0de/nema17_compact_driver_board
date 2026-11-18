#include <TMCStepper.h>


void tmc2209_setup(      // driver is disabled right after start, needs to call ENABLE
    TMC2209Stepper *driver,
    uint8_t dir_pin,
    uint8_t step_pin,
    uint8_t stdby_pin,
    uint8_t enn_pin,
    uint8_t rx_pin,
    uint8_t tx_pin,
    uint8_t diag_pin
);

void tmc2209_enable(int enn_pin);

void spin_motor(uint8_t step_pin, uint8_t dir_pin, uint8_t enn_pin, float angle_deg);

