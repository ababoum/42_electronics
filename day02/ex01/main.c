#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED_PIN PB1
#define DEBOUNCE_DELAY_MS 20
#define PRESCALER_T0 1024
#define PRESCALER_T1 1

typedef enum direction
{
    INCREMENTING,
    DECREMENTING
} t_direction;

uint32_t intensity;
uint32_t time1_max_ms;
uint32_t time2_max_ms;

void timer1_launch(void)
{
    // set up timer with prescaler = PRESCL / 1
    TCCR1B |= (1 << CS10);

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
    // set TOP value to the equivalent of 5 ms (200 steps of 5 ms in a 1 s period)
    ICR1 = (uint32_t)0xFFFF * 5 / time1_max_ms;
    // Note: The OC1A pin is also the LED pin - PB1
}

void timer0_launch(void)
{
    // set up timer with prescaler = 1024
    TCCR0B |= (1 << CS02) | (1 << CS00);
    // initialize counter
    TCNT0 = 0;
    // Enable CTC mode
    TCCR0A |= (1 << WGM01);
    // Enable CTC interrupt
    TIMSK0 |= (1 << OCIE0A);
    // set TOP/Compare value to the equivalent of 5 ms
    OCR0A = (int)(0xFF * 5 / time2_max_ms);
    // OCR0A = 79;
}

ISR(TIMER0_COMPA_vect)
{
    static t_direction direction = INCREMENTING;

    OCR1A = ICR1 * (0.01 * intensity);

    if (direction == INCREMENTING)
    {
        ++intensity;
        if (intensity == 100)
            direction = DECREMENTING;
    }
    else
    {
        --intensity;
        if (intensity == 0)
            direction = INCREMENTING;
    }
}

int main(void)
{
    DDRB |= (1 << LED_PIN); // Set LED_PIN as output
    intensity = 0;
    time1_max_ms = (1000 * 65535) / (F_CPU / PRESCALER_T1);         // = 8
    time2_max_ms = (uint32_t)(255) * 1000 / (F_CPU / PRESCALER_T0); // = 16

    timer1_launch();
    timer0_launch();
    sei(); // Enable global interrupts

    while (42)
    {
    }
}