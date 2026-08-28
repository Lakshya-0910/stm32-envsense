/*
 * main.c - EnvSense firmware entry point
 *
 * Reads a BME280 sensor every second over I2C1 and prints one CSV line
 * per reading over USART1 (115200 8N1):
 *
 *     millis,temp_c,pressure_hpa,humidity_pct
 *     1000,23.41,1013.02,44.87
 *
 * Pipe this straight into scripts/serial_logger.py to save it to a CSV
 * and plot it -- see the top-level README for the full walkthrough.
 */

#include "stm32f103_regs.h"
#include "i2c.h"
#include "uart.h"
#include "bme280.h"

static volatile uint32_t ms_ticks = 0;

/* Called by the SysTick interrupt every 1ms (see vector table in startup.s;
   this build uses a polled millis() instead, see delay_ms below, to keep
   the interrupt setup out of scope for a first project). */
static void clock_init_72mhz(void) {
    /* 2 wait states required for 72MHz flash access */
    FLASH->ACR = FLASH_ACR_LATENCY_2;

    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) { }

    RCC->CFGR = RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL9 | RCC_CFGR_PPRE1_DIV2;
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) { }

    RCC->CFGR = (RCC->CFGR & ~0x3UL) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & (0x3UL << 2)) != RCC_CFGR_SWS_PLL) { }
}

static void delay_ms(uint32_t ms) {
    /* crude busy-wait calibrated for 72MHz; swap for SysTick-based delay
       once you're comfortable adding the interrupt handler yourself */
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 9000; j++) { }
    }
}