#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4
#define DEBOUNCE_DELAY_MS 20

unsigned long counter;

// BUTTON SW1 (PD2) is INT0 / PCINT18
// BUTTON SW2 (PD4) is PCTINT20 / XCK / T0

void update_LEDS(void)
{
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

ISR(INT0_vect)
{
    if (counter <= 14)
    {
        ++counter;
        update_LEDS();
    }
    // Clear interruption flag
    EIFR |= (1 << INTF0);
}

ISR(PCINT2_vect)
{
    static int last_state = 0;
    if (counter >= 1 && last_state == 0)
    {
        --counter;
        last_state = 1;
        update_LEDS();
    }
    else
    {
        _delay_ms(DEBOUNCE_DELAY_MS);
        last_state = 0;
    }
    // Clear interruption flag
    PCIFR |= (1 << PCIF2);
}

int main(void)
{
    counter = 0;

    DDRB |= (1 << LED_PIN0) | (1 << LED_PIN1) | (1 << LED_PIN2) | (1 << LED_PIN3);

    EICRA |= (1 << ISC01); // The falling edge of INT0 generates an interrupt request.
    EIMSK |= (1 << INT0);  // Enable INT0 interrupt

    PCMSK2 |= (1 << PCINT20); // Enable PCINT20 (button SW2)
    PCICR |= (1 << PCIE2);    // Enable PCINT2 interrupt

    sei(); // Enable global interrupts

    while (42)
    {
    }
}