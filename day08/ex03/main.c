#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "libs/uart.h"
#include "libs/adc.h"

#define LED_D9_ADDR 0b0100111 // PORT 7 on the I2C expander
#define SS PB2
#define MOSI PB3
#define MISO PB4
#define SCK PB5
#define DDR_SPI DDRB
#define LED_COUNT 3

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
    uart_printnb_hex(cData);
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

int main(void)
{
    uart_init();
    SPI_MasterInit();
    init_adc(ADC_8_BIT);

    uint8_t level = 0;
    uint8_t map = 0;

    while (0x42)
    {
        level = read_adc_8b(ADC_POT);
        map = 0;
        if (level >= 0.33 * 0xFF)
            map |= 0b00000001;
        if (level >= 0.66 * 0xFF)
            map |= 0b00000010;
        if (level == 0xFF)
            map |= 0b00000100;
        set_multiple_leds(map, 1, 0xFF, 0x00, 0x00);
        _delay_ms(10);
    }
}