#include <util/delay.h>
#include "libs/uart.h"
#include "libs/i2c.h"

#define I2C_EXPANDER_ADDR 0b0100000
#define IO0_3_MASK   0b00001000 // LED D9
#define IO0_2_MASK   0b00000100 // LED D10
#define IO0_1_MASK   0b00000010 // LED D10

int main(void)
{
    uart_init();
    i2c_init();

    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 6); // select output register for P0 and set direction to output
    i2c_write(ACK, IO0_2_MASK);
    i2c_stop();

    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 2); // select output register for P0
    i2c_write(ACK, IO0_2_MASK);
    i2c_write(ACK, IO0_1_MASK);
    i2c_stop();

    while (1)
    {
    }
}