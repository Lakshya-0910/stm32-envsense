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
## Build

Requires the `arm-none-eabi-gcc` toolchain.

```bash
# Ubuntu/Debian
sudo apt-get install gcc-arm-none-eabi

# macOS
brew install --cask gcc-arm-embedded

cd firmware
make
```

This produces `envsense.elf` and `envsense.bin`. On this build the image
is ~3.3KB — comfortably inside the STM32F103C8T6's 64KB flash.

## Flash

With an ST-Link V2 (clone or genuine) connected to the SWD header (see
`hardware/connection_table.md` for pinout):

```bash
# using st-flash (from the stlink-tools package)
st-flash write envsense.bin 0x08000000

# or using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program envsense.bin 0x08000000 verify reset exit"
```

No ST-Link yet? A cheap clone (~$3-8) is worth buying — you'll reuse it
for every STM32 project after this one.

## Watch the output

Open a serial terminal at 115200 8N1 on whatever port your USB-UART
adapter enumerates as (see `hardware/connection_table.md` for TX/RX
wiring), or just pipe it straight into the logger:

```bash
python3 ../scripts/serial_logger.py --port /dev/ttyUSB0 --baud 115200 --out ../data/live_log.csv
```


## Design notes / things kept intentionally simple for a first project

- **Polling, not interrupts.** I2C and UART are both polled with timeout
  loops rather than using interrupts or DMA. This is easier to reason
  about for a first project; a natural "v2" improvement is to move UART
  TX to DMA and add a proper SysTick-based millis() instead of a
  busy-wait `delay_ms()`.
- **Forced mode, not normal mode, on the BME280.** The sensor sleeps
  between reads, which is lower power and simpler to reason about than
  continuous conversion.
- **Fixed-point math throughout**, per Bosch's published compensation
  formulas — no floating point unit is used, appropriate for a Cortex-M3.
