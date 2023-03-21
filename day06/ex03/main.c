#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define LED_BLUE PD3 // OC2B
#define LED_RED PD5 // 0C0B
#define LED_GREEN PD6 // OC0A

void init_rgb(void)
{
    // TIMER 0 (for red and green LED)
    TCCR0A = 0;
    TCCR0B = 0;
    // set up timer with prescaler = 1
    TCCR0B |= (1 << CS00);
    // Compare match to control OC0B (red LED) and OC0A (green LED)
    TCCR0A |= (1 << COM0B1) | (1 << COM0A1);
    // PWN mode 7
    TCCR0A |= (1 << WGM01) | (1 << WGM00);

    OCR0A = 0;
    OCR0B = 0;
    
    // TIMER 2 (for blue LED)
    TCCR2A = 0;
    TCCR2B = 0;
    // set up timer with prescaler = 1
    TCCR2B |= (1 << CS20);
    // Compare match to control OC2B (blue LED)
    TCCR2A |= (1 << COM2B1);
    // PWN mode 7
    TCCR2A |= (1 << WGM21) | (1 << WGM20);

    OCR2A = 0;
    
    // initialize counters
    TCNT0 = 0;
    TCNT2 = 0;
}


void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    OCR0B = r;
    OCR0A = g;
    OCR2B = b;
}

unsigned int ft_strlen(const char *s)
{
    unsigned int i = 0;
    while (s[i])
        i++;
    return i;
}

int main(void)
{
    uart_init();
    DDRD |= (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);
    init_rgb();
    PORTD = 0;

    while (0x1312)
    {
        uart_printstr("Enter a HEX color value (#RRGGBB):\n\r");

        char buf[42] = {0};
        char c = uart_rx();
        int i = 0;
        while (c != '\r')
        {
            if (c == 127 && i > 0)
            {
                buf[--i] = '\0';
                uart_printstr("\b \b");
            }
            else if (c != 127)
            {
                uart_tx(c);
                buf[i++] = c;
            }
            c = uart_rx();
        }

        uart_printstr("\r\n");

        if (ft_strlen(buf) != 7 || buf[0] != '#')
        {
            uart_printstr("Invalid input, try again.\n\r");
            continue;
        }

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        for (int i = 0; i < 6; i += 2)
        {
            if (buf[i + 1] >= '0' && buf[i + 1] <= '9')
                buf[i + 1] -= '0';
            else if (buf[i + 1] >= 'A' && buf[i + 1] <= 'F')
                buf[i + 1] -= 'A' - 10;
            else
            {
                uart_printstr("Invalid input, try again.\n\r");
                continue;
            }
        }

        r = (buf[1] << 4) | buf[2];
        g = (buf[3] << 4) | buf[4];
        b = (buf[5] << 4) | buf[6];

        set_rgb(r, g, b);
        uart_printstr("Color set.\n\r");
    }
}