#include "libs/uart.h"
#include "libs/spi.h"
#include "libs/adc.h"

#define BUTTON_PIN_SW1 PD2
#define BUTTON_PIN_SW2 PD4
#define DEBOUNCE_DELAY_MS 20

uint8_t red[3] = {0};
uint8_t green[3] = {0};
uint8_t blue[3] = {0};

uint8_t button_1_pressed = 0;
uint8_t button_2_pressed = 0;

uint8_t current_led = 1;
uint8_t current_primary_color = 0;

void change_current_led(void)
{
    current_led = 1 + (current_led % 3);
}

void change_current_primary_color(void)
{
    current_primary_color = (++current_primary_color % 3);
}

void update_current_primary_color(uint8_t value)
{
    switch (current_primary_color)
    {
    case 0:
        red[current_led - 1] = value;
        break;
    case 1:
        green[current_led - 1] = value;
        break;
    case 2:
        blue[current_led - 1] = value;
        break;
    }
}

int main(void)
{
    DDRD &= ~(1 << BUTTON_PIN_SW1) & ~(1 << BUTTON_PIN_SW2);
    PORTD |= (1 << BUTTON_PIN_SW1) | (1 << BUTTON_PIN_SW2);

    init_adc(ADC_8_BIT);
    uart_init();
    SPI_MasterInit();

    while (1)
    {
        // read the potentiometer value
        uint8_t potentiometer_value = read_adc_8b(ADC_POT);

        if (!(PIND & (1 << BUTTON_PIN_SW1))) // button SW1 is being pressed
        {
            if (!button_1_pressed)
            {
                change_current_led();
                current_primary_color = 0;
                button_1_pressed = 1;
            }
            _delay_ms(DEBOUNCE_DELAY_MS);
        }
        else
            button_1_pressed = 0;

        if (!(PIND & (1 << BUTTON_PIN_SW2))) // button SW2 is being pressed
        {
            if (!button_2_pressed)
            {
                update_current_primary_color(potentiometer_value);
                change_current_primary_color();
                button_2_pressed = 1;
            }
            _delay_ms(DEBOUNCE_DELAY_MS);
        }
        else
            button_2_pressed = 0;

        // update LEDs colors
        set_one_led(1, 1, red[0], green[0], blue[0]);
        set_one_led(2, 1, red[1], green[1], blue[1]);
        set_one_led(3, 1, red[2], green[2], blue[2]);
    }
}