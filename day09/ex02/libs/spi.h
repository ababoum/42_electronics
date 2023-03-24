#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SS PB2
#define MOSI PB3
#define MISO PB4
#define SCK PB5
#define DDR_SPI DDRB
#define LED_COUNT 3

void SPI_MasterInit(void);
void SPI_MasterTransmit(uint8_t cData);
void apa102_start(void);
void apa102_end(void);
void apa102_set_led(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue);
void set_one_led(uint8_t rank, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue);
void set_all_leds(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue);
void set_multiple_leds(uint8_t map, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue);

#endif