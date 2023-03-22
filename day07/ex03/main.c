#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define ADC_INTERNAL_TEMP 0b00001000 // ADC8 -> Internal temperature sensor

void init_adc(void)
{
    // Setting the prescaler to 128
    // If the clock frequency is too high, the ADC may not have enough time
    // to complete the conversion process before the next sample is taken,
    // leading to inaccurate readings.
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // 10-bit resolution, Vref = 1.1V
    ADMUX |= (1 << REFS0) | (1 << REFS1);
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

    uint16_t result;
    while (1)
    {
        result = read_adc_10b(ADC_INTERNAL_TEMP);

        // Display the results on UART
        uart_printnb(result - 342);
        uart_printstr("°C\r\n");
        _delay_ms(500);
    }
}