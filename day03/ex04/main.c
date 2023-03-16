#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#define UART_BAUDRATE 115200
#define MYUBRR 8 // Technically, F_CPU / (16L * UART_BAUDRATE) - 1 -> 7.68 =~ 8
#define LED_PIN0 PB0
#define LED_PIN1 PB1
#define LED_PIN2 PB2
#define LED_PIN3 PB4

const char *valid_username = "admin";
const char *valid_password = "admin";
char username[69] = {0};
char password[69] = {0};

void knight_lights(void)
{
    unsigned long spot = 1;
    int go_right = 1;

    DDRB |= (1 << LED_PIN0);
    DDRB |= (1 << LED_PIN1);
    DDRB |= (1 << LED_PIN2);
    DDRB |= (1 << LED_PIN3);

    while (1)
    {
        // Increment the counter every 0.5s
        _delay_ms(100);
        if (go_right)
        {
            spot <<= 1;
            if (spot == 8)
                go_right = 0;
        }
        else
        {
            spot >>= 1;
            if (spot == 1)
                go_right = 1;
        }

        // Light the LED according to the spot value
        if (spot & 0b0001)
            PORTB |= (1 << LED_PIN0);
        else
            PORTB &= ~(1 << LED_PIN0);

        if (spot & 0b0010)
            PORTB |= (1 << LED_PIN1);
        else
            PORTB &= ~(1 << LED_PIN1);

        if (spot & 0b0100)
            PORTB |= (1 << LED_PIN2);
        else
            PORTB &= ~(1 << LED_PIN2);

        if (spot & 0b1000)
            PORTB |= (1 << LED_PIN3);
        else
            PORTB &= ~(1 << LED_PIN3);
    }
}

int ft_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

uint32_t ft_strlen(const char *s)
{
    uint32_t i = 0;
    while (s[i])
        i++;
    return i;
}

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
    // Enable receiver and transmitter
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}

void ft_empty_buffers(void)
{
    for (int i = 0; i < 69; i++)
    {
        username[i] = '\0';
        password[i] = '\0';
    }
}

int main(void)
{
    int i;
    uart_init(MYUBRR);

    while (-0x69)
    {
        i = 0;
        uart_printstr("Enter your login:\r\n");
        uart_printstr("\tusername: ");
        char c = uart_rx();
        while (c != '\r')
        {
            if (c == 127 && i > 0)
            {
                username[--i] = '\0';
                uart_printstr("\b \b");
            }
            else if (c != 127)
            {
                uart_tx(c);
                username[i++] = c;
            }
            c = uart_rx();
        }
        uart_printstr("\r\n");
        uart_printstr("\tpassword: ");
        i = 0;
        c = uart_rx();
        while (c != '\r')
        {
            if (c == 127 && i > 0)
            {
                password[--i] = '\0';
                uart_printstr("\b \b");
            }
            else if (c != 127)
            {
                uart_tx('*');
                password[i++] = c;
            }
            c = uart_rx();
        }

        if (ft_strcmp(username, valid_username) == 0 &&
            ft_strcmp(password, valid_password) == 0)
        {
            uart_printstr("\n");
            uart_printstr("\r");
            uart_printstr("Welcome to the secret area, ");
            uart_printstr(username);
            uart_printstr("!\r\n");
            uart_printstr("Have a nice day!\r\n");
            knight_lights();
            break;
        }
        else
        {
            uart_printstr("\n");
            uart_printstr("\r");
            uart_printstr("Invalid username or password!\r\n");
            uart_printstr("Try again!\r\n");
            ft_empty_buffers();
        }
    }

    return 0;
}