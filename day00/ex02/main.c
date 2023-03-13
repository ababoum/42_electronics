#include <avr/io.h>

// Make the LED blink on PB0 with 1 Hz frequency
// 0 for low, 1 for high
int main(void)
{
    unsigned long i;

    // Set the bit corresponding to PB0 to 1 in the Data Direction Register
    DDRB |= (1 << PB0);
    while (1)
    {
        i = 0;
        PORTB ^= (1 << PB0);

        // each loop needs 8 clock ticks
        while (i < F_CPU / 8)
        {
            ++i;
        }
    }
}