#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <util/twi.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define SQL_FREQ 100000UL
#define PRESCALER 1

uint8_t error_status;

#define ERROR(status_code, msg)     \
    uart_printstr("Error: ");       \
    uart_printstr("0x");            \
    uart_printnb_hex(status_code);  \
    uart_printstr(" --- ");         \
    uart_printstr(msg);             \
    uart_printstr("\r\n");          \
    error_status = 1;               \
    return;

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
void i2c_start(mode_t mode);
void i2c_write(unsigned char data);
void i2c_read(uint8_t ack);

#endif