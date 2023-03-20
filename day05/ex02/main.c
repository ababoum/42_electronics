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

#define EEPROM_ADDRESS 0
#define EEPROM_COUNTER_OFFSET 0
#define MAGIC_NUMBER (uint8_t)42

typedef enum {
    false,
    true
} bool;

uint8_t counters[4];
uint8_t idx = 0;
int button_1_pressed = 0;
int button_2_pressed = 0;

// Buffer is the destination, EEPROM is the source
bool safe_eeprom_read(void *buffer, size_t offset, size_t length)
{
    uint8_t *ptr = (uint8_t *)buffer;
    uint8_t *magic_ptr = (uint8_t *)EEPROM_ADDRESS + offset;
    uint8_t *counter_ptr = (uint8_t *)EEPROM_ADDRESS + offset + 1;

    if (eeprom_read_byte(magic_ptr) != MAGIC_NUMBER)
    {
        uart_printstr("Read failed: EEPROM is not initialized\n\r");
        return false;
    }

    for (size_t i = 0; i < length; ++i)
        ptr[i] = eeprom_read_byte(counter_ptr + i);

    return true;
}

// Buffer is the source, EEPROM is the destination
bool safe_eeprom_write(void *buffer, size_t offset, size_t length)
{
    uint8_t *ptr = (uint8_t *)buffer;
    uint8_t *magic_ptr = (uint8_t *)EEPROM_ADDRESS + offset;
    uint8_t *counter_ptr = (uint8_t *)EEPROM_ADDRESS + offset + 1;

    if (eeprom_read_byte(magic_ptr) != MAGIC_NUMBER)
    {
        uart_printstr("Write failed: EEPROM is not initialized or corrupted\n\r");
        return false;
    }

    for (size_t i = 0; i < length; ++i)
    {
        // To avoid unnecessary writes
        if (ptr[i] != eeprom_read_byte(counter_ptr + i))
            eeprom_update_byte(counter_ptr + i, ptr[i]);
    }
    return true;
}

void increment_EEPROM_counter(void)
{
    ++counters[idx];
    counters[idx] %= 16;
    eeprom_update_byte(
        (uint8_t *)(EEPROM_ADDRESS + EEPROM_COUNTER_OFFSET) + idx + 1,
        counters[idx]);

}

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

    DDRD &= ~(1 << BUTTON_PIN_SW2);
    PORTD |= (1 << BUTTON_PIN_SW2);

    // Load counter value from EEPROM, or initialize it
    if (safe_eeprom_read(counters,
                        0,
                        4) == false)
    {
        uart_printstr("Initializing EEPROM...\n\r");
        eeprom_update_byte((uint8_t *)(
            EEPROM_ADDRESS + EEPROM_COUNTER_OFFSET),
            MAGIC_NUMBER);

        for (size_t i = 0; i < 4; ++i)
        {
            counters[i] = 0;
            eeprom_update_byte((uint8_t *)(EEPROM_ADDRESS + EEPROM_COUNTER_OFFSET) + i + 1, 0);
        }
        uart_printstr("EEPROM initialized!\n\r");
    }

    while (0xDECAF)
    {
        monitor_buttons();
        display_counter_LED();
    }
}