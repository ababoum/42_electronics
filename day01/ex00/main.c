#include <avr/io.h>

#define LED_PIN PB1
#define PRESCL 1024
#define TIME_MAX_COUNT_MS 1000 * 65535 / (F_CPU / PRESCL)

void timer1_launch(void)
{
    // set up timer with prescaler = PRESCL / 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCCR1B &= ~(1 << CS11);
    
    // CTC mode
    TCCR1B |= (1 << WGM12);
    
    // Toggle OC1A on compare match
    TCCR1A |= (1 << COM1A0);
    TCCR1A &= ~(1 << COM1A1);

    // initialize counter
    TCNT1 = 0;
    // set compare value to the equivalent of 500 ms
    OCR1A = 0xFFFF / (TIME_MAX_COUNT_MS / 500);

    // Note: The OC1A pin is also the LED pin - PB1
}

int main(void)
{
    DDRB |= (1 << LED_PIN);  // Set LED_PIN as output

    timer1_launch();

    while (42) {}
}