# Connection Table

Use this as your source of truth while drawing the schematic in KiCad.
Every net listed here should become a wire (or labeled net) in your schematic.

## STM32F103C8T6 <-> BME280 (I2C1)

| STM32 Pin | Signal      | BME280 Pin | Notes                                |
|-----------|-------------|------------|---------------------------------------|
| PB6       | I2C1_SCL    | SCK/SCL    | Add 4.7k pull-up to 3.3V              |
| PB7       | I2C1_SDA    | SDI/SDA    | Add 4.7k pull-up to 3.3V              |
| 3.3V      | VDD         | VDD        | BME280 is 3.3V logic, do NOT use 5V   |
| GND       | GND         | GND        |                                        |
| 3.3V      | -           | SDO        | Tie SDO to GND for address 0x76 (used by firmware), or to 3.3V for 0x77 |
| 3.3V      | -           | CSB        | Tie CSB to 3.3V to force I2C mode (not SPI) |

## STM32F103C8T6 <-> USB-UART header (for programming logs / debug)

| STM32 Pin | Signal | Header Pin | Notes                        |
|-----------|--------|------------|-------------------------------|
| PA9       | USART1_TX | RX (on adapter) | Board TX -> adapter RX |
| PA10      | USART1_RX | TX (on adapter) | Board RX -> adapter TX |
| 3.3V      | -      | 3.3V       | Only if adapter can supply power |
| GND       | -      | GND        | Common ground, always connect |

## STM32F103C8T6 <-> SWD programming header (ST-Link)

| STM32 Pin | Signal   | SWD Header Pin |
|-----------|----------|-----------------|
| PA13      | SWDIO    | Pin 2           |
| PA14      | SWCLK    | Pin 4           |
| 3.3V      | VDD      | Pin 1           |
| GND       | GND      | Pin 3           |
| NRST      | Reset    | Pin 5 (optional)|