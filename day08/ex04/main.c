#include "libs/uart.h"
#include "libs/spi.h"

static uint32_t wheel(uint32_t pos)
{
    pos = 255 - pos;
    if (pos < 85)
    {
        return (255 - pos * 3) << 16 | (pos * 3) << 8;
    }
    if (pos < 170)
    {
        pos -= 85;
        return (pos * 3) << 16 | (255 - pos * 3) << 0;
    }
    pos -= 170;
    return (pos * 3) << 8 | (255 - pos * 3) << 0;
}

int main(void)
{
    uart_init();
    SPI_MasterInit();

    uint8_t v = 0;
    uint32_t colors;

    while (0x1312)
    {
        uart_printstr("Enter a HEX color value (#RRGGBBDX), DX being the LED to change:\n\r");

        unsigned char buf[42] = {0};
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

        if (!ft_strcmp(buf, "#FULLRAINBOW"))
        {
            goto full_rainbow;
        }

        if (ft_strlen(buf) != 8 || buf[0] != '#')
        {
            uart_printstr("Invalid input (Length or first character), try again.\n\r");
            continue;
        }

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        for (int i = 1; i < 7; i++)
        {
            if (buf[i] >= '0' && buf[i] <= '9')
                buf[i] -= '0';
            else if (buf[i] >= 'A' && buf[i] <= 'F')
                buf[i] -= 'A' - 10;
            else
            {
                uart_printstr("Invalid input (Color), try again.\n\r");
                continue;
            }
        }

        r = (buf[1] << 4) | buf[2];
        g = (buf[3] << 4) | buf[4];
        b = (buf[5] << 4) | buf[6];

        if (buf[7] < '6' || buf[7] > '8')
        {
            uart_printstr("Invalid input (DX), try again.\n\r");
            continue;
        }

        uint8_t rank = buf[7] - '0' - 5;
        set_one_led(rank, 1, r, g, b);
        uart_printstr("Color set.\n\r");
        continue;

    full_rainbow:


        while (1)
        {
            colors = wheel(v++);
            set_all_leds(1, colors >> 16, colors >> 8, colors);
            _delay_ms(10);
        }
    }
}