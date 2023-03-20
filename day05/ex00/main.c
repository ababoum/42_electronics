#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "uart.h"

#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4

#define BUTTON_PIN_SW1 PD2
#define BUTTON_PIN_SW2 PD4
#define DEBOUNCE_DELAY_MS 20

#define EEPROM_COUNTER_ADDRESS 255

void display_counter_LED(unsigned long counter)
{
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

void update_EEPROM_counter(unsigned long *counter, int delta)
{
    *counter += delta;
    eeprom_update_block(counter, (unsigned long *)EEPROM_COUNTER_ADDRESS, sizeof(unsigned long));
}

void monitor_buttons(int *button_1_pressed, int *button_2_pressed, unsigned long *counter)
{
    if (!(PIND & (1 << BUTTON_PIN_SW1))) // button SW1 is being pressed
    {
        if (!*button_1_pressed && *counter <= 14)
        {
            uart_printstr("Button 1 pressed\n\r");
            update_EEPROM_counter(1);
            *button_1_pressed = 1;
        }
        _delay_ms(DEBOUNCE_DELAY_MS);
    }
    else
        *button_1_pressed = 0;
    
    if (!(PIND & (1 << BUTTON_PIN_SW2))) // button SW2 is being pressed
    {
        if (!*button_2_pressed && *counter >= 1)
        {
            uart_printstr("Button 2 pressed\n\r");
            update_EEPROM_counter(-1);
            *button_2_pressed = 1;
        }
        _delay_ms(DEBOUNCE_DELAY_MS);
    }
    else
        *button_2_pressed = 0;
}

int main()
{
    unsigned long counter = 0;
    int button_1_pressed = 0;
    int button_2_pressed = 0;

    uart_init();

    DDRB |= (1 << LED_PIN0) | (1 << LED_PIN1) |
            (1 << LED_PIN2) | (1 << LED_PIN3);

    DDRD &= ~(1 << BUTTON_PIN_SW1);
    PORTD |= (1 << BUTTON_PIN_SW1);

    DDRD &= ~(1 << BUTTON_PIN_SW2);
    PORTD |= (1 << BUTTON_PIN_SW2);

    // Init EEPROM address register
    EEARH = 0;
    EEARL = EEPROM_COUNTER_ADDRESS;

    // Load counter value from EEPROM
    counter = 0;
    eeprom_read_block(&counter, (unsigned long *)EEPROM_COUNTER_ADDRESS, sizeof(counter));

    while (0xDECAF)
    {
        monitor_buttons(&button_1_pressed, &button_2_pressed, &counter);
        display_counter_LED(counter);
    }
}