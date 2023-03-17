#include <avr/io.h>
#include <util/twi.h>

// Source: https://www.electronicwings.com/avr-atmega/atmega1632-i2c

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define SQL_FREQ 100000UL
#define PRESCALER 1

#define UART_BAUDRATE 115200
#define MYUBRR 8 // Technically, F_CPU / (16L * UART_BAUDRATE) - 1 -> 7.68 =~ 8
#define BASE_16 "0123456789ABCDEF"

#define ERROR(msg)         \
    uart_printstr(msg);    \
    uart_printstr("\r\n"); \
    i2c_stop();            \
    error_status = 1;      \
    return;

#define ECHO_STATUS(status_code, msg) \
    uart_printstr("Status code: ");   \
    uart_printstr("0x");              \
    uart_printnb_hex(status_code);    \
    uart_printstr(" --- ");           \
    uart_printstr(msg);               \
    uart_printstr("\r\n");

#define MT_SLA_ACK 0x18  // Master transmitter, SLA+W has been transmitted; ACK has been received
#define MT_DATA_ACK 0x28 // Master transmitter, data has been transmitted; ACK has been received
#define DATA 0x33        // Command to start measurement
#define SLA_W 0x38       // Temperature sensor AHT20 (U3)
#define SLA_R 0x39       // Temperature sensor AHT20 (U3)

int error_status;

void uart_tx(char c)
{
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    // Put data into buffer, sends the data
    UDR0 = c;
}

void uart_printstr(const char *str)
{
    while (*str)
        uart_tx(*str++);
}

char uart_rx(void)
{
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)))
        ;
    // Get and return received data from buffer
    return UDR0;
}

void uart_printnb(unsigned int nb)
{
    if (nb >= 10)
        uart_printnb(nb / 10);
    uart_tx(nb % 10 + '0');
}

void uart_printnb_hex(unsigned int nb)
{
    if (nb >= 16)
        uart_printnb_hex(nb / 16);
    uart_tx(BASE_16[nb % 16]);
}

void uart_init(unsigned int ubrr)
{
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;

    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}

void i2c_init(void)
{
    // Set SCL to 100 KHz, prescaler = 1
    TWSR = 0;
    TWBR = ((F_CPU / SQL_FREQ) - 16) / (2 * PRESCALER);
    // Set our slave address
    TWAR = SLA_W;
}

void i2c_stop(void)
{
    // Send stop condition
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

    // Wait till stop condition is transmitted
    while (TWCR & (1 << TWSTO))
        ;

    ECHO_STATUS(0xF8, "Stop condition transmitted");
}

void i2c_start(void)
{
    uint8_t status;

    // Send start condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    // Wait for TWINT flag set. This indicates that the
    // start condition has been transmitted.
    while (!(TWCR & (1 << TWINT)))
        ;
    status = TWSR & 0xF8;
    if (status != TW_START)
    {
        ERROR("Start not received. Exiting...");
    }
    else
    {
        ECHO_STATUS(status, "Start condition transmitted");
    }
    TWDR = SLA_W;
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT flag set. This indicates that the SLA+W has
    // been transmitted, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    status = TWSR & 0xF8;
    if (status & MT_SLA_ACK)
    {
        ECHO_STATUS(status, "SLA+W transmitted, ACK received");
    }
    else if (status & TW_MT_SLA_NACK)
    {
        ECHO_STATUS(status, "SLA+W transmitted, NACK received");
    }
    else
    {
        ERROR("SLA+W not transmitted. Exiting...");
    }
}

void i2c_write(unsigned char data)
{
    // Load data into TWDR register. Clear TWINT bit in TWCR to start
    // transmission of data
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT flag set. This indicates that the data has been
    // transmitted, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    // Check value of TWI status register. Mask prescaler bits. If
    // status different from MT_DATA_ACK go to ERROR
    if ((TWSR & 0xF8) != MT_DATA_ACK)
        ERROR("Data not received. Exiting...");
}

void i2c_read(void)
{
    // Clear TWINT bit in TWCR to start transmission of data
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT flag set. This indicates that the data has been
    // received, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    // Check value of TWI status register. Mask prescaler bits. If
    // status different from MT_DATA_ACK go to ERROR
    if ((TWSR & 0xF8) != MT_DATA_ACK)
        ERROR("Data not received. Exiting...");
}

int main(void)
{
    error_status = 0;
    uart_init(MYUBRR);
    i2c_init();
    if (error_status)
        return 1;
    i2c_start();
    if (error_status)
        return 1;
    i2c_stop();

    while (1)
    {
        // Do nothing
    }
}