#include <util/delay.h>
#include "libs/uart.h"
#include "libs/i2c.h"

#define I2C_EXPANDER_ADDR 0b0100000
#define IO0_3_MASK   0b00001000 // LED D9
#define IO0_2_MASK   0b00000100 // LED D10
#define IO0_1_MASK   0b00000010 // LED D11
#define IO0_0_MASK   0b00000001 // LED D11

uint8_t button_sw3_pressed = 0;
uint8_t counter = 0;

void display_i2c_leds(uint8_t counter)
{
    uint8_t leds = 0;

    if (counter & 0b00000001)
        leds |= IO0_3_MASK;
    if (counter & 0b00000010)
        leds |= IO0_2_MASK;
    if (counter & 0b00000100)
        leds |= IO0_1_MASK;

    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 2);
    i2c_write(ACK, ~leds);
    i2c_stop();
}

int main(void)
{
    uart_init();
    i2c_init();

    // Set IO0_3, IO0_2, and IO0_1 as output (SW3 will be input by default)
    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 6);
    i2c_write(ACK, ~(IO0_3_MASK | IO0_2_MASK | IO0_1_MASK)); 
    i2c_stop();

    while (1)
    {
        // Read button value
        i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
        i2c_write(ACK, 0);
        i2c_start(I2C_EXPANDER_ADDR, TW_READ);
        uint8_t reg0 = i2c_read(NACK);
        i2c_stop();

        // Check if SW3 is pressed
        if (reg0 & IO0_0_MASK)
        {
            if (!button_sw3_pressed)
            {
                button_sw3_pressed = 1;
                counter = (counter + 1) % 8;
            }
        }
        else
        {
            button_sw3_pressed = 0;
        }
        display_i2c_leds(counter);
    }
}