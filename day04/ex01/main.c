#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

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

#define CHECK_ERROR()      \
    if (error_status)      \
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
#define SLA_R ((SLA_ADDR << 1) | 1)

int error_status;
unsigned char last_read_byte;

typedef enum mode {
    READ,
    WRITE
} mode_t;

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

void uart_printnb_bin(unsigned int nb)
{
    for (int i = 0; i < 8; i++)
    {
        if (nb & (1 << (7 - i)))
            uart_tx('1');
        else
            uart_tx('0');
    }
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
    TWAR = SLA_ADDR;
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

void i2c_start(mode_t mode)
{
    uint8_t status;

    // Send start condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    // Wait for TWINT flag set. This indicates that the
    // start condition has been transmitted.
    while (!(TWCR & (1 << TWINT)))
        ;
    status = TWSR & 0xF8;
    
    mode == READ ? uart_printstr("[READ MODE] ") : uart_printstr("[WRITE MODE] ");
    ECHO_STATUS(status, "Start condition transmitted");
    
    if (mode == READ)
    {
        TWDR = SLA_R;
    }
    else
    {
        TWDR = SLA_W;
    }
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT flag set. This indicates that the SLA+W has
    // been transmitted, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    status = TWSR & 0xF8;
    if (status & TW_MT_SLA_ACK)
    {
        ECHO_STATUS(status, "SLA+W/R transmitted, ACK received");
    }
    else if (status & TW_MT_SLA_NACK)
    {
        ECHO_STATUS(status, "SLA+W/R transmitted, NACK received");
    }
    else
    {
        // ERROR("SLA+W/R not transmitted. Exiting...");
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
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
    {
        ERROR("Data not received. Exiting...");
    }
    else
    {
        ECHO_STATUS(TWSR & 0xF8, "Data transmitted, ACK received");
        uart_printstr("Data sent to slave: 0x");
        uart_printnb_hex(data);
        uart_printstr("\r\n");
    }	
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
    if ((TWSR & 0xF8) == TW_MT_DATA_ACK || (TWSR & 0xF8) == TW_MR_DATA_NACK)
    {
        last_read_byte = TWDR;
        ECHO_STATUS(TWSR & 0xF8, "Data received, ACK/NACK transmitted to slave");
    }
    else
    {
        ERROR("Data not received. Exiting...");
    }
}

void send_ack(void)
{
    // Send ACK to the sensor
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

    // Wait for TWINT flag set. This indicates that the data has been
    // received, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;
}

void request_raw_data(void)
{
    // Start measurement
    i2c_start(WRITE);
    i2c_write(MEASURE_CMD);
    i2c_write(DATA0_CMD);
    i2c_write(DATA1_CMD);
    i2c_stop();

    _delay_ms(100);

    // Read data from the sensor
    i2c_start(READ);

    i2c_read();
    uart_printstr("State: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr(" (");
    uart_printnb_bin(last_read_byte);
    uart_printstr(")");
    uart_printstr("\r\n");
    send_ack();

    i2c_read();
    uart_printstr("Humidity data 1: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr("\r\n");
    send_ack();

    i2c_read();
    uart_printstr("Humidity data 2: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr("\r\n");
    send_ack();

    i2c_read();
    uart_printstr("Humidity/Temp data: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr("\r\n");
    send_ack();

    i2c_read();
    uart_printstr("Temp data 1: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr("\r\n");
    send_ack();

    i2c_read();
    uart_printstr("Temp data 2: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr("\r\n");
    send_ack();

    i2c_read();
    uart_printstr("CRC data: ");
    uart_printnb_hex(last_read_byte);
    uart_printstr("\r\n");

    i2c_stop();
}

int main(void)
{
    error_status = 0;
    uart_init(MYUBRR);
    i2c_init();
    CHECK_ERROR();
check_status:
    i2c_start(WRITE);
    CHECK_ERROR();
    i2c_write(STATUS_CMD);
    CHECK_ERROR();
    // i2c_stop();

    i2c_start(READ);
    // CHECK_ERROR();
    i2c_read();
    CHECK_ERROR();
    if (last_read_byte & 0b10000000)
    {
        uart_printstr("Device is busy\n\r");
        _delay_ms(100);
        goto check_status;
    }
    else
    {
        uart_printstr("Device is ready\n\r");
        if (last_read_byte & (1 << 3))
            uart_printstr("Device is calibrated\r\n");
        else
            uart_printstr("Warning: Device is not calibrated\r\n");
    }
    i2c_stop();

    while (1)
    {
        _delay_ms(2000);
        request_raw_data();
    }
}