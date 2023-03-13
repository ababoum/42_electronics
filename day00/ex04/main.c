#include <avr/io.h>
#include <util/delay.h>

#define BUTTON_PIN PD2
#define LED_PIN PB0
#define DEBOUNCE_DELAY_MS 10

// Make the LED (PB0) turn on when button SW1 (PD2) is pressed
// 0 for low, 1 for high
// 0 for input, 1 for output
int main(void)
{
    // Set the bit corresponding to PB0 to 1 [output] in the Data Direction Register
    DDRB |= (1 << LED_PIN);
    // Set the bit corresponding to PD2 to 0 [input] in the Data Direction Register
    DDRD &= ~(1 << BUTTON_PIN);
    // Enable pull-up resistor
    PORTD |= (1 << BUTTON_PIN);

    // Start with a LED on
    PORTB |= (1 << LED_PIN);

    while (1)
    {
        if (!(PIND & (1 << BUTTON_PIN))) // button is being pressed
        {
            PORTB ^= (1 << LED_PIN);
            _delay_us(1000 * DEBOUNCE_DELAY_MS);
        }
    }
}