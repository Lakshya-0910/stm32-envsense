# Firmware

Bare-metal C firmware for the STM32F103C8T6, written directly against the
register map (no HAL/CMSIS vendor libraries) so every line is traceable to
the reference manual. Reads a BME280 over I2C1 once a second and streams
CSV over USART1.

## Layout

```
firmware/
├── inc/                 headers (register map, drivers)
│   ├── stm32f103_regs.h  hand-written peripheral register definitions
│   ├── i2c.h
│   ├── uart.h
│   └── bme280.h
├── src/
│   ├── startup.s         vector table + reset handler
│   ├── main.c             clock init, main loop
│   ├── i2c.c              I2C1 polling driver
│   ├── uart.c             USART1 polling driver
│   └── bme280.c           sensor driver + Bosch compensation formulas
├── linker.ld              memory layout (64KB flash / 20KB RAM)
└── Makefile
```
