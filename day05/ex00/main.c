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

typedef enum {
    DEC,
    INC
} t_counter_direction;

unsigned long counter = 0;
int button_1_pressed = 0;
int button_2_pressed = 0;

void display_counter_LED(void)
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

void update_EEPROM_counter(t_counter_direction dir)
{
    uart_printstr("Updating EEPROM counter\n\r");
    uart_printstr("Counter value: ");
    uart_printnb(counter);
    uart_printstr("\n\r");
    if (dir == DEC && counter == 0UL)
        return;
    else if (dir == INC && counter + 1 == 16UL)
        return;
    
    dir == INC ? counter++ : counter--;
    eeprom_update_block(&counter, (unsigned long *)EEPROM_COUNTER_ADDRESS, sizeof(unsigned long));
}

void monitor_buttons(void)
{
    if (!(PIND & (1 << BUTTON_PIN_SW1))) // button SW1 is being pressed
    {
        if (!button_1_pressed)
        {
            uart_printstr("Button 1 pressed\n\r");
            update_EEPROM_counter(INC);
            button_1_pressed = 1;
        }
        _delay_ms(DEBOUNCE_DELAY_MS);
    }
    else
        button_1_pressed = 0;
    
    if (!(PIND & (1 << BUTTON_PIN_SW2))) // button SW2 is being pressed
    {
        if (!button_2_pressed)
        {
            uart_printstr("Button 2 pressed\n\r");
            update_EEPROM_counter(DEC);
            button_2_pressed = 1;
        }
        _delay_ms(DEBOUNCE_DELAY_MS);
    }
    else
        button_2_pressed = 0;
}

void calibrate_counter(void)
{
    if (counter > 15)
        counter = 15;
    eeprom_update_block(&counter, (unsigned long *)EEPROM_COUNTER_ADDRESS, sizeof(unsigned long));
}

int main()
{
    uart_init();

    DDRB |= (1 << LED_PIN0) | (1 << LED_PIN1) |
            (1 << LED_PIN2) | (1 << LED_PIN3);

    DDRD = ~(1 << BUTTON_PIN_SW1) & ~(1 << BUTTON_PIN_SW2);
    PORTD |= (1 << BUTTON_PIN_SW1);
    PORTD |= (1 << BUTTON_PIN_SW2);

    // Init EEPROM address register
    // EEARH = 0;
    // EEARL = EEPROM_COUNTER_ADDRESS;

    // Load counter value from EEPROM
    eeprom_read_block(&counter, (unsigned long *)EEPROM_COUNTER_ADDRESS, sizeof(counter));
    calibrate_counter();

    while (0xDECAF)
    {
        monitor_buttons();
        display_counter_LED();
    }
}