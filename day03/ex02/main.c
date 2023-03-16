#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define UART_BAUDRATE 115200
#define MYUBRR 8 // Technically, F_CPU / (16L * UART_BAUDRATE) - 1 -> 7.68 =~ 8

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

void uart_printnb(unsigned int nb)
{
    if (nb >= 10)
        uart_printnb(nb / 10);
    uart_tx(nb % 10 + '0');
}

char uart_rx(void)
{
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)))
        ;
    // Get and return received data from buffer
    return UDR0;
}

void uart_init(unsigned int ubrr)
{
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;

    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter.
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}

int main(void)
{
    uart_init(MYUBRR);

    while (1993)
    {
        char c = uart_rx();
        uart_printstr("You typed: ");
        uart_tx(c);
        uart_printstr(", ASCII code: ");
        uart_printnb(c);
        uart_printstr("\r\n");
    }
}