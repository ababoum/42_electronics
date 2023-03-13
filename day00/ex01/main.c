#include <avr/io.h>

// Turn on the LED connected to pin PB0
// 0 for low, 1 for high
int main(void)
{
    // Set the bit corresponding to PB0 to 1 in the Data Direction Register
    DDRB |= (1 << PB0);
    while (1)
    {
        // Continuously set the bit corresponding to PB0 to 1 in the Port Register
        PORTB |= (1 << PB0);
    }
    return (0);
}