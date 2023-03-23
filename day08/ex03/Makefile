CC = avr-gcc
FCPU = 16000000UL
NAME = main
MCU = atmega328p
SRCS = main.c libs/uart.c libs/adc.c

ifeq ($(shell uname -r),4.4.0-19041-Microsoft)
	DEVICE = /dev/ttyS6
else
	DEVICE = /dev/ttyUSB0
endif
PROGRAMMER = arduino

all: hex flash

hex: $(SRCS)
	$(CC) -mmcu=$(MCU) -O1 -DF_CPU=$(FCPU) -o $(NAME).bin $(SRCS)
	avr-objcopy -O ihex $(NAME).bin $(NAME).hex

flash: $(NAME).hex
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(DEVICE) -U flash:w:$(NAME).hex

clean: 
	rm -f $(NAME).bin $(NAME).hex