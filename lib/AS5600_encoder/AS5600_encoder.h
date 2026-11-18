#pragma once
#include <Arduino.h>

#define AS5600_RESOLUTION   12

#define AS5600_ADC_BITS 12
#define AS5600_ADC_MAX  ((1 << AS5600_ADC_BITS) - 1)

float read_shaft_angle_deg(uint8_t adc_pin);