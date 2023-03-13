#include <avr/io.h>

// Turn on the LED connected to pin PB0
int main(void)
{
    DDRB |= (1 << PB0);
    while (1)
    {
        PORTB |= (1 << PB0);
    }
    return (0);
}