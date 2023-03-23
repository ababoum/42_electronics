#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "libs/uart.h"
#include "libs/i2c.h"

#define LED_D9_ADDR 0b0100111 // PORT 7 on the I2C expander


int main(void)
{
    uart_init();
    i2c_init();
}