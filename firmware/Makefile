TARGET = envsense
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU = -mcpu=cortex-m3 -mthumb
CFLAGS = $(CPU) -Wall -Wextra -O2 -ffunction-sections -fdata-sections -std=c11 -Iinc
LDFLAGS = $(CPU) -Tlinker.ld -nostartfiles -Wl,--gc-sections -Wl,-Map=$(TARGET).map

SRCS = src/startup.s src/main.c src/i2c.c src/uart.c src/bme280.c
OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CPU) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f src/*.o $(TARGET).elf $(TARGET).bin $(TARGET).map

.PHONY: all clean
