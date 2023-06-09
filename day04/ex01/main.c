#include "main.h"
#include <avr/io.h>
#include <util/delay.h>

int error_status;
unsigned char last_read_byte;

typedef enum mode
{
    READ,
    WRITE
} mode_t;

enum ack
{
    NACK = 0,
    ACK = 1
};

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
        ERROR("SLA+W/R not transmitted. Exiting...");
    }
}

void i2c_write(unsigned char data)
{
    // Load data into TWDR register. Clear TWINT bit in TWCR to start
    // transmission of data
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

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

void i2c_read(uint8_t ack)
{
    uint8_t status;

    // Clear TWINT bit in TWCR to start transmission of data
    TWCR = (1 << TWINT) | (1 << TWEN) | (ack << TWEA);

    // Wait for TWINT flag set. This indicates that the data has been
    // received, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    status = TWSR & 0xF8;
    // Check value of TWI status register. Mask prescaler bits. If
    // status different from MT_DATA_(N)ACK go to ERROR
    if (status & TW_MT_DATA_ACK || status & TW_MR_DATA_NACK)
    {
        last_read_byte = TWDR;
        ECHO_STATUS(TWSR & 0xF8, "Data received, ACK/NACK transmitted to slave");
    }
    else
    {
        ERROR("Data not received. Exiting...");
    }
}

void request_raw_data(void)
{
    // Start measurement
    i2c_start(WRITE);
    i2c_write(MEASURE_CMD);
    i2c_write(DATA0_CMD);
    i2c_write(DATA1_CMD);
    // i2c_stop();

    _delay_ms(200);

    // Read data from the sensor
    i2c_start(READ);

    i2c_read(ACK);
    uart_printstr("State: ");
    GREEN_ON();
    uart_printnb_hex(last_read_byte);
    uart_printstr(" (");
    uart_printnb_bin(last_read_byte);
    uart_printstr(")");
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_read(ACK);
    uart_printstr("Humidity data 1: ");
    RED_ON();
    uart_printnb_hex(last_read_byte);
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_read(ACK);
    uart_printstr("Humidity data 2: ");
    RED_ON();
    uart_printnb_hex(last_read_byte);
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_read(ACK);
    uart_printstr("Humidity/Temp data: ");
    RED_ON();
    uart_printnb_hex(last_read_byte);
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_read(ACK);
    uart_printstr("Temp data 1: ");
    RED_ON();
    uart_printnb_hex(last_read_byte);
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_read(ACK);
    uart_printstr("Temp data 2: ");
    RED_ON();
    uart_printnb_hex(last_read_byte);
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_read(NACK);
    uart_printstr("CRC data: ");
    GREEN_ON();
    uart_printnb_hex(last_read_byte);
    COLOR_OFF();
    uart_printstr("\r\n");

    i2c_stop();
}

int main(void)
{
    error_status = 0;
    uart_init(MYUBRR);
    uart_printstr("Program started\r\n");

    i2c_init();
    CHECK_ERROR();
check_status:
    i2c_start(WRITE);
    i2c_write(STATUS_CMD);
    i2c_stop();
    CHECK_ERROR();

    _delay_ms(100);

    i2c_start(READ);
    i2c_read(0);
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