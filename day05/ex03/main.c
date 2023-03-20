#include <avr/io.h>
#include <avr/eeprom.h>
#include "uart.h"

#define EEPROM_ADDRESS 0
#define MAGIC_NUMBER (uint8_t)42

typedef enum {
    false,
    true
} bool;


// Length is the size of the data in bytes
typedef struct {
    uint16_t id;
    uint16_t length;
} eepromalloc_t;


size_t get_id_offset(uint16_t id)
{
    uint8_t *cursor = (uint8_t *)EEPROM_ADDRESS + 1;
    eepromalloc_t *block = (eepromalloc_t *)cursor;
    size_t offset = 0;

    while (block->id != id)
    {
        cursor += sizeof(eepromalloc_t) + block->length;
        if (cursor >= (uint8_t *)1024)
            return 0;
        block = (eepromalloc_t *)cursor;
    }
    return cursor - (uint8_t *)EEPROM_ADDRESS;
}

// Write to the EEPROM (destination) the content of the buffer (source)
bool eepromalloc_write(uint16_t id, void *buffer, uint16_t length)
{
    if (get_id_offset(id) != 0)
        return false;
}

// Read from the EEPROM (source) at the given id, into the buffer (destination)
bool eepromalloc_read(uint16_t id, void *buffer, uint16_t length)
{

}

bool eepromalloc_free(uint16_t id)
{

}

void reset_eeprom(void)
{
    for (uint16_t i = 0; i < 1024; i++)
        eeprom_write_byte((uint8_t *)i, 0);
}

int main(void)
{
    uart_init();
    
    // Check if the EEPROM is initialized
    if (eeprom_read_byte((uint8_t *)EEPROM_ADDRESS) != MAGIC_NUMBER)
    {
        uart_printstr("EEPROM is not initialized, resetting it\n\r");
        reset_eeprom();
        eeprom_write_byte((uint8_t *)EEPROM_ADDRESS, MAGIC_NUMBER);
    }
}