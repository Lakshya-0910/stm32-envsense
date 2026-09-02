# Design Notes

Short write-up of decisions and trade-offs — useful both for future-me and
for anyone (recruiter, interviewer) trying to understand why the board
looks the way it does.

## Why STM32F103C8T6?

- Extremely well documented (the "Blue Pill" is one of the most-cloned
  dev boards ever, so tutorials/errata/community knowledge are abundant).
- Cheap (~$2-3) and easy to hand-solder (LQFP-48, 0.5mm pitch — doable
  with a fine-tip iron and patience, or hot air).
- 72MHz Cortex-M3 with hardware I2C and USART is comfortably more than
  this project needs, leaving headroom to add features later (SD card
  logging, a second sensor, BLE module, etc.) without re-spinning the MCU.

## Why BME280 over an IMU for a first project?

- I2C only, no SPI complexity, and a single well-documented compensation
  algorithm (published by Bosch, same across every BME280 driver in the
  world).
- Produces continuously changing, physically meaningful data (ambient
  temperature/humidity/pressure) without needing motion — good for
  generating logs and plots to demonstrate the board works, even sitting
  still on a desk.
- Sets up a natural "v2" project: swap in a BMI270 IMU or add a second
  I2C sensor, since the bus and driver pattern already exist.

## Why bare-metal register access instead of STM32 HAL/CubeMX?

- Trade-off, made deliberately: HAL would get a working prototype faster,
  but writing directly to registers forces understanding of what's
  actually happening (clock trees, peripheral enable bits, I2C state
  machine) rather than treating the MCU as a black box.
- Downside: more code, more room for subtle bugs, no vendor abstraction
  if porting to a different STM32 family later. For a learning project
  this trade-off favors bare-metal; for a larger/production project HAL
  or CubeMX-generated code would likely be the better call.

## Known limitations / v2 ideas

- Polling-based I2C/UART instead of interrupts or DMA — fine at 1 sample/sec,
  would need rework for higher sample rates.
- No onboard USB (relies on an external USB-TTL adapter) — v2 could add
  a CP2102 or use the STM32's built-in USB peripheral with a DFU/CDC
  bootloader instead.
- No local storage — every reading has to be captured live over serial.
  A µSD card + SPI would let the board log standalone, away from a PC.
- Single sensor. The I2C bus and connection pattern here generalizes
  directly to adding more I2C sensors on the same two wires (each with
  its own address).
