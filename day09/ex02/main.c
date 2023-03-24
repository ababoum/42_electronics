#include <util/delay.h>
#include "libs/uart.h"
#include "libs/i2c.h"

#define I2C_EXPANDER_ADDR 0b0100000
#define CA4     0b10000000
#define LCD_a   0b00000001
#define LCD_b   0b00000010
#define LCD_c   0b00000100
#define LCD_d   0b00001000
#define LCD_e   0b00010000
#define LCD_f   0b00100000
#define LCD_g   0b01000000
#define LCD_dp  0b10000000

int main(void)
{
    uart_init();
    i2c_init();

    // Set all port 1 as outputs
    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 7);
    i2c_write(ACK, 0x00);
    i2c_stop();

    // Set all port 0 to input, except for CA4
    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 6);
    i2c_write(ACK, 0x01);
    i2c_stop();

    // Select CA4
    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 2);
    i2c_write(ACK, ~CA4);
    i2c_stop();

    // 4th character
    i2c_start(I2C_EXPANDER_ADDR, TW_WRITE);
    i2c_write(ACK, 3);
    i2c_write(ACK, LCD_a | LCD_b | LCD_g | LCD_e | LCD_d);
    i2c_stop();

    while (042)
    {
    }
}