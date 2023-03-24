#include "spi.h"

void SPI_MasterInit(void)
{
    // Set MOSI and SCK output
    DDR_SPI |= (1 << MOSI) | (1 << SCK) | (1 << SS);
    // Enable SPI, Master, set clock rate fck/16
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void SPI_MasterTransmit(uint8_t cData)
{
    // Start transmission
    SPDR = cData;
    // Wait for transmission complete
    while (!(SPSR & (1 << SPIF)))
        ;
}

void apa102_start(void)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        SPI_MasterTransmit(0x00);
    }
}

void apa102_end(void)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        SPI_MasterTransmit(0xFF);
    }
}

void apa102_set_led(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    SPI_MasterTransmit((0b11100000) | (brightness & 0b00011111));
    SPI_MasterTransmit(blue);
    SPI_MasterTransmit(green);
    SPI_MasterTransmit(red);
}

void set_one_led(uint8_t rank, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    apa102_start();

    if (rank % 4 == 0)
    {
        apa102_set_led(0, 0x00, 0x00, 0x00);
        apa102_set_led(0, 0x00, 0x00, 0x00);
        apa102_set_led(0, 0x00, 0x00, 0x00);
        apa102_end();
        return;
    }

    for (uint8_t i = 1; i < 4; i++)
    {
        if (rank % 4 == i)
            apa102_set_led(brightness, red, green, blue);
        else
            apa102_set_led(0, 0x00, 0x00, 0x00);
    }

    apa102_end();
}

void set_all_leds(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    apa102_start();

    for (uint8_t i = 0; i < 3; i++)
    {
        apa102_set_led(brightness, red, green, blue);
    }

    apa102_end();
}

void set_multiple_leds(uint8_t map, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    apa102_start();

    if (map & 0b00000001)
        apa102_set_led(brightness, red, green, blue);
    else
        apa102_set_led(0, 0x00, 0x00, 0x00);

    if (map & 0b00000010)
        apa102_set_led(brightness, red, green, blue);
    else
        apa102_set_led(0, 0x00, 0x00, 0x00);

    if (map & 0b00000100)
        apa102_set_led(brightness, red, green, blue);
    else
        apa102_set_led(0, 0x00, 0x00, 0x00);

    apa102_end();
}