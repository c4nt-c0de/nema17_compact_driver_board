#include "AS5600_encoder.h"

float read_shaft_angle_deg(uint8_t adc_pin)
{
    int adc = analogRead(adc_pin);
    return (adc * 360.0f) / AS5600_ADC_MAX;
}