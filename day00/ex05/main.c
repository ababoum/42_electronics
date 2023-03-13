#include <avr/io.h>
#include <util/delay.h>

#define BUTTON_PIN_SW1 PD2
#define BUTTON_PIN_SW2 PD4
#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4
#define DEBOUNCE_DELAY_MS 500

// 0 for low, 1 for high
// 0 for input, 1 for output
int main(void)
{
    unsigned long counter = 0;

    DDRB |= (1 << LED_PIN0);
    DDRB |= (1 << LED_PIN1);
    DDRB |= (1 << LED_PIN2);
    DDRB |= (1 << LED_PIN3);

    DDRD &= ~(1 << BUTTON_PIN_SW1);
    PORTD |= (1 << BUTTON_PIN_SW1);

    DDRD &= ~(1 << BUTTON_PIN_SW2);
    PORTD |= (1 << BUTTON_PIN_SW2);

    while (1)
    {
        // increment the counter
        if (!(PIND & (1 << BUTTON_PIN_SW1))) // button is being pressed
        {
            if (counter <= 14)
                ++counter;
            _delay_ms(DEBOUNCE_DELAY_MS);
        }
        else if (!(PIND & (1 << BUTTON_PIN_SW2))) // button is being pressed
        {
            if (counter >= 1)
                --counter;
            _delay_ms(DEBOUNCE_DELAY_MS);
        }

        // Light the LED according to the counter
        if (counter & 0b0001)
            PORTB |= (1 << LED_PIN0);
        else
            PORTB &= ~(1 << LED_PIN0);
        
        if (counter & 0b0010)
            PORTB |= (1 << LED_PIN1);
        else
            PORTB &= ~(1 << LED_PIN1);

        if (counter & 0b0100)
            PORTB |= (1 << LED_PIN2);
        else
            PORTB &= ~(1 << LED_PIN2);
        
        if (counter & 0b1000)
            PORTB |= (1 << LED_PIN3);
        else
            PORTB &= ~(1 << LED_PIN3);
    }
}