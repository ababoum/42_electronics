#include <avr/io.h>

// Turn on the LED connected to pin PB0
// 0 for low, 1 for high
int main(void)
{
    // Set the bit corresponding to PB0 to 1 [output] in the Data Direction Register
    DDRB |= (1 << PB0);

    // Set the bit corresponding to PB0 to 1 in the Port Register
    PORTB |= (1 << PB0);
}