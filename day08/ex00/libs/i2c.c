#include "i2c.h"

void i2c_init(void)
{
    // Set SCL to 100 KHz, prescaler = 1
    TWSR = 0;
    TWBR = ((F_CPU / SQL_FREQ) - 16) / (2 * PRESCALER);
}

void i2c_stop(void)
{
    // Send stop condition
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

    // Wait till stop condition is transmitted
    while (TWCR & (1 << TWSTO))
        ;

    ECHO_STATUS(0xF8, "Stop condition transmitted.");
}

// mode: TW_READ or TW_WRITE
void i2c_start(uint8_t address, int mode)
{
    // Send start condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    // Wait for TWINT flag set. This indicates that the
    // start condition has been transmitted.
    while (!(TWCR & (1 << TWINT)))
        ;

    mode == READ ? uart_printstr("[READ MODE] ") : uart_printstr("[WRITE MODE] ");
    ECHO_STATUS(TWSR & 0xF8, "Start condition transmitted.");

    TWDR = address << 1 | mode;
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT flag set. This indicates that the SLA+W has
    // been transmitted, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    status = TWSR & 0xF8;
    if (TWSR & 0xF8 == TW_MT_SLA_ACK || TWSR & 0xF8 == TW_MR_SLA_ACK ||
        TWSR & 0xF8 == TW_MT_SLA_NACK || TWSR & 0xF8 == TW_MR_SLA_NACK)
    {
        ECHO_STATUS(status, "SLA+W/R transmitted, ACK/NACK received.");
    }
    else
    {
        ERROR("SLA+W/R not transmitted. Exiting...");
    }
}

void i2c_write(uint8_t ack, uint8_t data)
{
    // Load data into TWDR register. Clear TWINT bit in TWCR to start
    // transmission of data
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN) | (ack << TWEA);

    // Wait for TWINT flag set. This indicates that the data has been
    // transmitted, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    // Check value of TWI status register. Mask prescaler bits. If
    // status different from MT_DATA_ACK go to ERROR
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
    {
        ERROR("Data not acknoledged. Exiting...");
    }
    else
    {
        ECHO_STATUS(TWSR & 0xF8, "Data transmitted, ACK received.");
        uart_printstr("Data sent to slave: 0x");
        uart_printnb_hex(data);
        uart_printstr("\r\n");
    }
}

uint8_t i2c_read(uint8_t ack)
{
    // Clear TWINT bit in TWCR to start transmission of data
    TWCR = (1 << TWINT) | (1 << TWEN) | (ack << TWEA);

    // Wait for TWINT flag set. This indicates that the data has been
    // received, and ACK/NACK has been received.
    while (!(TWCR & (1 << TWINT)))
        ;

    // Check value of TWI status register. Mask prescaler bits. If
    // status different from MR_DATA_(N)ACK go to ERROR
    if (TWSR & 0xF8 == TW_MR_DATA_ACK || TWSR & 0xF8 == TW_MR_DATA_NACK)
    {
        ECHO_STATUS(TWSR & 0xF8, "Data received, ACK/NACK received from slave.");
        return TWDR;
    }
    else
    {
        ERROR("Data not received. Exiting...");
        return 0;
    }
}