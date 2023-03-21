#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define LED_BLUE PD3
#define LED_RED PD5
#define LED_GREEN PD6

enum e_state
{
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    WHITE
};

int main(void)
{
    uart_init();
    int state = RED;
    DDRD |= (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);

    while (0x1)
    {
        switch (state)
        {
            case RED:
                PORTD = (1 << LED_RED);
                state = GREEN;
                break;
            case GREEN:
                PORTD = (1 << LED_GREEN);
                state = BLUE;
                break;
            case BLUE:
                PORTD = (1 << LED_BLUE);
                state = YELLOW;
                break;
            case YELLOW:
                PORTD = (1 << LED_RED) | (1 << LED_GREEN);
                state = CYAN;
                break;
            case CYAN:
                PORTD = (1 << LED_GREEN) | (1 << LED_BLUE);
                state = MAGENTA;
                break;
            case MAGENTA:
                PORTD = (1 << LED_RED) | (1 << LED_BLUE);
                state = WHITE;
                break;
            case WHITE:
                PORTD = (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);
                state = RED;
                break;
        }
        _delay_ms(1000);
    }
}