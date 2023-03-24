#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <util/twi.h>
#include "uart.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define SQL_FREQ 100000UL
#define PRESCALER 1

enum {
    ACK = 1,
    NACK = 0
};

uint8_t error_status;

#define ERROR(status_code, msg)     \
    uart_printstr("Error: ");       \
    uart_printstr("0x");            \
    uart_printnb_hex(status_code);  \
    uart_printstr(" --- ");         \
    uart_printstr(msg);             \
    uart_printstr("\r\n");          \
    error_status = 1;

#define CHECK_ERROR() \
    if (error_status) \
        return 1;

#define ECHO_STATUS(status_code, msg) \
    uart_printstr("Status code: ");   \
    uart_printstr("0x");              \
    uart_printnb_hex(status_code);    \
    uart_printstr(" --- ");           \
    uart_printstr(msg);               \
    uart_printstr("\r\n");

void i2c_init(void);
void i2c_stop(void);
void i2c_start(uint8_t address, int mode);
void i2c_write(uint8_t ack, uint8_t data);
uint8_t i2c_read(uint8_t ack);

#endif