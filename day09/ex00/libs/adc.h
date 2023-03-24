#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

#define ADC_INTERNAL_TEMP 0b00001000 // ADC8 -> Internal temperature sensor
#define ADC_POT 0b00000000           // ADC0 -> Potentiometer

typedef enum adc_mode
{
    ADC_8_BIT,
    ADC_10_BIT
} adc_mode;

void init_adc(adc_mode mode);
uint8_t read_adc_8b(uint8_t channel);
uint16_t read_adc_10b(uint8_t channel);

#endif