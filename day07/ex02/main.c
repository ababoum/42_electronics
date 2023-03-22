#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define ADC_POT 0b00000000 // ADC0
#define ADC_LDR 0b00000001 // ADC1 -> Photoresistor
#define ADC_NTC 0b00000010 // ADC2 -> Thermistor

void init_adc(void)
{
    // Setting the prescaler to 128
    // If the clock frequency is too high, the ADC may not have enough time
    // to complete the conversion process before the next sample is taken,
    // leading to inaccurate readings.
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // 10-bit resolution, Vref = Vcc
    ADMUX |= (1 << REFS0);
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

int main(void)
{
    uart_init();
    init_adc();

    uint16_t results[3] = {0};
    while (1)
    {
        results[0] = read_adc_10b(ADC_POT);
        results[1] = read_adc_10b(ADC_LDR);
        results[2] = read_adc_10b(ADC_NTC);

        // Display the results on UART
        uart_printnb(results[0]);
        uart_printstr(", ");
        uart_printnb(results[1]);
        uart_printstr(", ");
        uart_printnb(results[2]);
        uart_printstr("\r\n");
        _delay_ms(20);
    }
}