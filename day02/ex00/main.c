#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED_PIN PB1
#define BUTTON_PIN PD2
#define DEBOUNCE_DELAY_MS 20

ISR(INT0_vect)
{
    static int last_state = 0;

    if (last_state == 0)
    {
        PORTB ^= (1 << LED_PIN);
        _delay_ms(DEBOUNCE_DELAY_MS);
        last_state = 1;
    }
    else
    {
        _delay_ms(DEBOUNCE_DELAY_MS);
        last_state = 0;
    }
    // Clear interruption flag
    EIFR |= (1 << INTF0);
}

int main(void)
{
    DDRB |= (1 << LED_PIN);     // Set LED_PIN as output
    // DDRD &= ~(1 << BUTTON_PIN); // Set BUTTON_PIN as input
    // PORTD |= (1 << BUTTON_PIN); // Enable pull-up resistor on BUTTON_PIN

    EICRA |= (1 << ISC00); // Any logical change on INT0 generates an interrupt request
    EIMSK |= (1 << INT0);  // Enable INT0 interrupt

    sei(); // Enable global interrupts

    while (42)
    {
    }
}