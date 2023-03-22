#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define ADC_POT 0b00000000 // ADC0

int main(void)
{
    // Configure ADC
    ADMUX = (1 << REFS0); // AVcc as reference
    // 8-bit resolution
    ADMUX |= (1 << ADLAR);
    // Setting the prescaler to 128
    // If the clock frequency is too high, the ADC may not have enough time
    // to complete the conversion process before the next sample is taken, 
    // leading to inaccurate readings.
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Configure to read ADC_POT
    ADMUX |= ADC_POT;

    // Display the result on UART
    uart_init();

    uint8_t result = 0;
    while (1)
    {
        // Trigger a single conversion
        ADCSRA |= (1 << ADSC) | (1 << ADEN);

        // Read value of RV1 potentiometer
        while (ADCSRA & (1 << ADSC))
            ;
        result = ADCH;

        // Display the result on UART
        uart_printnb_hex(result);
        uart_printstr("\r\n");
        _delay_ms(20);
    }
}