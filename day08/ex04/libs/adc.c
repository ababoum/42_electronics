#include "adc.h"

void init_adc(adc_mode mode)
{
    // Setting the prescaler to 128
    // If the clock frequency is too high, the ADC may not have enough time
    // to complete the conversion process before the next sample is taken,
    // leading to inaccurate readings.
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // 8-bit or 10-bit resolution, Vref = Vcc
    if (mode == ADC_8_BIT)
        ADMUX |= (1 << REFS0) | (1 << ADLAR);
    else if (mode == ADC_10_BIT)
        ADMUX |= (1 << REFS0) | (1 << REFS1) | (1 << ADLAR);
}

uint8_t read_adc_8b(uint8_t channel)
{
    // Select ADC channel
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Start ADC conversion
    ADCSRA |= (1 << ADSC) | (1 << ADEN);
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC))
        ;
    // Return ADC result
    return ADCH;
}

uint16_t read_adc_10b(uint8_t channel)
{
    // Select ADC channel
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Start ADC conversion
    ADCSRA |= (1 << ADSC) | (1 << ADEN);
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC))
        ;
    // Return ADC result
    return ADC & 0x3FF;
}
