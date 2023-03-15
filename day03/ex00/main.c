#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define BAUD 9600
#define MYUBRR F_CPU / (16L * BAUD) - 1

void uart_init(unsigned int ubrr)
{

    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter.
    UCSR0B |= (1 << TXEN0);
}

void uart_tx(unsigned char c)
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
        uart_tx('z');
        _delay_ms(100);
    }
}