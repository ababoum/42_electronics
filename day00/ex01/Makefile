CC = avr-gcc

NAME = main

SRC = main.c

all: hex flash

hex: $(SRC)
	$(CC) -mmcu=atmega328p -DF_CPU=8000000UL -o $(NAME).bin $(SRC)
	avr-objcopy -O ihex $(NAME).bin $(NAME).hex

flash: $(NAME).hex
	avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -U flash:w:$(NAME).hex

clean: 
	rm -f $(NAME).bin $(NAME).hex