#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define LED_BLUE PD3 // OC2B
#define LED_RED PD5 // 0C0B
#define LED_GREEN PD6 // OC0A

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

void wheel(uint8_t pos) {
    pos = 255 - pos;
    if (pos < 85) {
        set_rgb(255 - pos * 3, 0, pos * 3);
    } else if (pos < 170) {
        pos = pos - 85;
        set_rgb(0, pos * 3, 255 - pos * 3);
    } else {
        pos = pos - 170;
        set_rgb(pos * 3, 255 - pos * 3, 0);
    }
}


int main(void)
{
    uart_init();
    DDRD |= (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);
    init_rgb();
    PORTD = 0;

    uint8_t i = 0;
    while (0x1312)
    {
        wheel(i++);
        _delay_ms(10);
    }
}