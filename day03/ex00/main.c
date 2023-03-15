#include <avr/io.h>
#include <util/delay.h>

#define BAUD 115200
#define MYUBRR F_CPU / 16 / BAUD - 1

void uart_init(unsigned int ubrr)
{

    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Set MSPI mode of operation and SPI data mode 0
    UCSR0C = (1 << UMSEL01) | (1 << UMSEL00) | (0 << UCPHA0) | (0 << UCPOL0);
    // Enable receiver and transmitter.
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void uart_tx(char c)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    /* Put data into buffer, sends the data */
    UDR0 = c;
}

int main(void)
{
    uart_init(MYUBRR);

    while (1)
    {
        uart_tx('Z');
        _delay_ms(100);
    }
}