#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdlib.h>
#include "uart.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define SQL_FREQ 100000UL
#define PRESCALER 1

#define ERROR(msg)         \
    uart_printstr(msg);    \
    uart_printstr("\r\n"); \
    error_status = 1;      \
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

#define MEASURE_CMD 0xAC // Command to start measurement
#define STATUS_CMD 0x71  // Command to read status
#define DATA0_CMD 0x33   // Command to read data (param1)
#define DATA1_CMD 0x00   // Command to read data (param2)
#define SLA_ADDR 0x38    // Temperature sensor AHT20 (U3)
#define SLA_W (SLA_ADDR << 1)
#define SLA_R (SLA_ADDR << 1) | 1
#define NACK 0
#define ACK 1