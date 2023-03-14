#include <avr/io.h>

#define LED_PIN PB1
#define PRESCL 1024
#define TIME_MAX_COUNT_MS 1000 * 65535 / (F_CPU / PRESCL)

void timer1_launch(void)
{
    // set up timer with prescaler = PRESCL / 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);

    // PWM mode
    unsigned int mode[] = {1, 1, 1, 0};
    // A -> WG10 and WG11
    // B -> WG12 and WG13
    int flags[] = {WGM13, WGM12, WGM11, WGM10};

    for (int i = 0; i < 4; i++)
    {
        if (mode[i] == 0)
        {
            if (i == 2 || i == 3)
                TCCR1A &= ~(1 << flags[i]);
            else
                TCCR1B &= ~(1 << flags[i]);
        }
        else
        {
            if (i == 2 || i == 3)
                TCCR1A |= (1 << flags[i]);
            else
                TCCR1B |= (1 << flags[i]);
        }
    }

    TCCR1A &= ~(1 << COM1A0);
    TCCR1A |= (1 << COM1A1);

    // initialize counter
    TCNT1 = 0;
    // set TOP value to the equivalent of 1000 ms
    ICR1 = 0xFFFF / (TIME_MAX_COUNT_MS / 1000);
    // set compare value to the equivalent of 100 ms
    OCR1A = 0xFFFF / (TIME_MAX_COUNT_MS / 100);

    // Note: The OC1A pin is also the LED pin - PB1
}

int main(void)
{
    DDRB |= (1 << LED_PIN); // Set LED_PIN as output

    timer1_launch();

    while (42)
    {
    }
}