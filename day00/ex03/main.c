#include <avr/io.h>

// Make the LED (PB0) turn on when button SW1 (PD2) is pressed
// 0 for low, 1 for high
int main(void)
{

    // Set the bit corresponding to PB0 to 1 in the Data Direction Register
    DDRB |= (1 << PB0);
    // Set the bit corresponding to PD2 to 0 in the Data Direction Register
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    while (1)
    {
        // Set the bit corresponding to PB0 to 1 in the Port Register when SW1 is pressed
        while (!(PIND & (1 << PD2)))
        {
            PORTB |= (1 << PB0);
        }
        PORTB &= ~(1 << PB0);
    }
}