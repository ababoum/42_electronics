#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define ADC_POT 0b00000000 // ADC0 -> Potentiometer
#define LED_BLUE PD3       // OC2B
#define LED_RED PD5        // 0C0B
#define LED_GREEN PD6      // OC0A

#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4

void display_led_counter(uint8_t level)
{
    if (level >= 255 / 4)
        PORTB |= (1 << LED_PIN0);
    else
        PORTB &= ~(1 << LED_PIN0);

    if (level >= 255 / 2)
        PORTB |= (1 << LED_PIN1);
    else
        PORTB &= ~(1 << LED_PIN1);

    if (level >= 3 * 255 / 4)
        PORTB |= (1 << LED_PIN2);
    else
        PORTB &= ~(1 << LED_PIN2);

    if (level == 255)
        PORTB |= (1 << LED_PIN3);
    else
        PORTB &= ~(1 << LED_PIN3);
}

void init_rgb(void)
{
    // TIMER 0 (for red and green LED)
    TCCR0A = 0;
    TCCR0B = 0;
    // set up timer with prescaler = 1
    TCCR0B |= (1 << CS00);
    // Compare match to control OC0B (red LED) and OC0A (green LED)
    TCCR0A |= (1 << COM0B1) | (1 << COM0A1);
    // PWN mode 7
    TCCR0A |= (1 << WGM01) | (1 << WGM00);

    OCR0A = 0;
    OCR0B = 0;

    // TIMER 2 (for blue LED)
    TCCR2A = 0;
    TCCR2B = 0;
    // set up timer with prescaler = 1
    TCCR2B |= (1 << CS20);
    // Compare match to control OC2B (blue LED)
    TCCR2A |= (1 << COM2B1);
    // PWN mode 7
    TCCR2A |= (1 << WGM21) | (1 << WGM20);

    OCR2A = 0;

    // initialize counters
    TCNT0 = 0;
    TCNT2 = 0;
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    OCR0B = r;
    OCR0A = g;
    OCR2B = b;
}

void wheel(uint8_t pos)
{
    pos = 255 - pos;
    if (pos < 85)
    {
        set_rgb(255 - pos * 3, 0, pos * 3);
    }
    else if (pos < 170)
    {
        pos = pos - 85;
        set_rgb(0, pos * 3, 255 - pos * 3);
    }
    else
    {
        pos = pos - 170;
        set_rgb(pos * 3, 255 - pos * 3, 0);
    }
}

void init_adc(void)
{
    // Setting the prescaler to 128
    // If the clock frequency is too high, the ADC may not have enough time
    // to complete the conversion process before the next sample is taken,
    // leading to inaccurate readings.
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // 8-bit resolution, Vref = Vcc
    ADMUX |= (1 << REFS0) | (1 << ADLAR);
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

int main(void)
{
    uart_init();
    init_adc();
    init_rgb();

    DDRD |= (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);

    uint8_t result;
    while (1)
    {
        result = read_adc_8b(ADC_POT);
        uart_printnb(result);
        uart_printstr("\r\n");
        
        wheel(result);
        display_led_counter(result);
        _delay_ms(20);
    }
}