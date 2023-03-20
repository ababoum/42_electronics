#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "uart.h"

#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4

#define BUTTON_PIN_SW1 PD2 // to switch between counters
#define BUTTON_PIN_SW2 PD4 // to increment the selected counter
#define DEBOUNCE_DELAY_MS 20

#define EEPROM_COUNTER_ADDRESS 0


uint8_t counters[4];
uint8_t idx = 0;
int button_1_pressed = 0;
int button_2_pressed = 0;

void display_counter_LED(void)
{
    if (counters[idx] & 0b0001)
        PORTB |= (1 << LED_PIN0);
    else
        PORTB &= ~(1 << LED_PIN0);

    if (counters[idx] & 0b0010)
        PORTB |= (1 << LED_PIN1);
    else
        PORTB &= ~(1 << LED_PIN1);

    if (counters[idx] & 0b0100)
        PORTB |= (1 << LED_PIN2);
    else
        PORTB &= ~(1 << LED_PIN2);

    if (counters[idx] & 0b1000)
        PORTB |= (1 << LED_PIN3);
    else
        PORTB &= ~(1 << LED_PIN3);
}

void increment_EEPROM_counter(void)
{
    uart_printstr("Updating EEPROM counter #");
    uart_printnb(idx);
    uart_printstr("\n\r");
    
    ++(counters[idx]);
    counters[idx] %= 16;
    eeprom_update_block(counters + idx,
                        (uint8_t *)(EEPROM_COUNTER_ADDRESS) + idx,
                        sizeof(uint8_t));
}

void monitor_buttons(void)
{
    if (!(PIND & (1 << BUTTON_PIN_SW1))) // switch between counters
    {
        if (!button_1_pressed)
        {
            idx = (idx + 1) % 4;
            button_1_pressed = 1;
            uart_printstr("Focus on counter #");
            uart_printnb(idx);
            uart_printstr("\n\r");
        }
        _delay_ms(DEBOUNCE_DELAY_MS);
    }
    else
        button_1_pressed = 0;
    
    if (!(PIND & (1 << BUTTON_PIN_SW2))) // increment current counter
    {
        if (!button_2_pressed)
        {
            uart_printstr("Incrementing counter #");
            uart_printnb(idx);
            uart_printstr("\n\r");

            increment_EEPROM_counter();
            
            uart_printstr("Counter value: ");
            uart_printnb(counters[idx]);
            uart_printstr("\n\r");

            button_2_pressed = 1;
        }
        _delay_ms(DEBOUNCE_DELAY_MS);
    }
    else
        button_2_pressed = 0;
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
    eeprom_read_block(counters,
                        (uint8_t *)EEPROM_COUNTER_ADDRESS,
                        sizeof(uint8_t) * 4);

    while (0xDECAF)
    {
        monitor_buttons();
        display_counter_LED();
    }
}