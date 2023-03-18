#include "main.h"

int error_status;
uint8_t last_read_byte;

typedef enum mode
{
    READ,
    WRITE
} mode_t;

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

void print_temp_hum_state(uint8_t *raw_data)
{
    uint32_t temp, hum;
    double temp_c, hum_c;

    hum = raw_data[1];
    hum <<= 8;
    hum += raw_data[2];
    hum <<= 4;
    hum += (raw_data[3] & 0xF0) >> 4;

    temp = raw_data[3] & 0x0F;
    temp <<= 8;
    temp += raw_data[4];
    temp <<= 8;
    temp += raw_data[5];

    // hum_c = (double)hum / (1ULL << 20);
    hum_c = (double)hum / 1048576.0 * 100.0;
    // temp_c = (double)temp / (1ULL << 20) * 200 - 50;
    temp_c = (double)temp / 1048576.0 * 200.0 - 50.0;

    char temp_str[100] = {0};
    char hum_str[100] = {0};

    dtostrf(temp_c, 4, 2, temp_str);
    dtostrf(hum_c, 4, 2, hum_str);

    uart_printstr("Temperature: ");
    uart_printstr(temp_str);
    uart_printstr("°C, Humidity: ");
    uart_printstr(hum_str);
    uart_printstr("%\r\n");
}

void request_raw_data(void)
{
    uint8_t raw_data[6] = {0};

    // Start measurement
    i2c_start(WRITE);
    i2c_write(MEASURE_CMD);
    i2c_write(DATA0_CMD);
    i2c_write(DATA1_CMD);
    i2c_stop();

    _delay_ms(200);

    // Read data from the sensor
    i2c_start(READ);

    // State
    i2c_read(ACK);
    raw_data[0] = last_read_byte;
    if (!(raw_data[0] & (1 << 7)))
    {
        _delay_ms(200);
    }
    for (int i = 1; i < 6; i++)
    {
        i2c_read(ACK);
        raw_data[i] = last_read_byte;
    }

    i2c_read(NACK);
    // We don't do anything with the CRC byte for now

    i2c_stop();

    GREEN_ON();
    print_temp_hum_state(raw_data);
    COLOR_OFF();
}

int main(void)
{
    error_status = 0;
    uart_init(MYUBRR);
    uart_printstr("========== Program started ==========\r\n");

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