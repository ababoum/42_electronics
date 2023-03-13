#include <avr/io.h>
#include <util/delay.h>


#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4


int main(void)
{
    unsigned long spot = 1;
    int go_right = 1;

    DDRB |= (1 << LED_PIN0);
    DDRB |= (1 << LED_PIN1);
    DDRB |= (1 << LED_PIN2);
    DDRB |= (1 << LED_PIN3);

    while (1)
    {
        // Increment the counter every 0.5s
        _delay_ms(100);
        if (go_right)
        {
            spot <<= 1;
            if (spot == 8)
                go_right = 0;
        }
        else
        {
            spot >>= 1;
            if (spot == 1)
                go_right = 1;
        }

        // Light the LED according to the spot value
        if (spot & 0b0001)
            PORTB |= (1 << LED_PIN0);
        else
            PORTB &= ~(1 << LED_PIN0);
        
        if (spot & 0b0010)
            PORTB |= (1 << LED_PIN1);
        else
            PORTB &= ~(1 << LED_PIN1);

        if (spot & 0b0100)
            PORTB |= (1 << LED_PIN2);
        else
            PORTB &= ~(1 << LED_PIN2);
        
        if (spot & 0b1000)
            PORTB |= (1 << LED_PIN3);
        else
            PORTB &= ~(1 << LED_PIN3);
    }
}