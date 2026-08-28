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

int main(void) {
    clock_init_72mhz();
    uart1_init(115200);
    i2c1_init();

    uart1_write_str("EnvSense STM32F103 + BME280 booting...\r\n");

    if (!bme280_init()) {
        uart1_write_str("ERROR: BME280 not found on I2C bus. Check wiring.\r\n");
        while (1) { }
    }

    uart1_write_str("millis,temp_c,pressure_hpa,humidity_pct\r\n");

    uint32_t millis = 0;
    while (1) {
        bme280_reading_t r;
        bme280_read(&r);

        uart1_write_int((int32_t)millis);
        uart1_write_char(',');

        /* temperature: centi-C -> "XX.XX" */
        uart1_write_int(r.temperature_centi_c / 100);
        uart1_write_char('.');
        int32_t t_frac = r.temperature_centi_c % 100;
        if (t_frac < 0) t_frac = -t_frac;
        if (t_frac < 10) uart1_write_char('0');
        uart1_write_int(t_frac);
        uart1_write_char(',');

        /* pressure: Q24.8 Pa -> hPa with 2 decimal places */
        uint32_t pa = r.pressure_pa_q24_8 >> 8;
        uint32_t pa_frac = ((r.pressure_pa_q24_8 & 0xFF) * 100) / 256;
        uart1_write_int((int32_t)(pa / 100));
        uart1_write_char('.');
        uint32_t hpa_frac = ((pa % 100) * 100 + pa_frac) / 100;
        if (hpa_frac < 10) uart1_write_char('0');
        uart1_write_int((int32_t)hpa_frac);
        uart1_write_char(',');

        /* humidity: Q22.10 %RH -> "XX.XX" */
        uint32_t h_whole = r.humidity_q22_10 >> 10;
        uint32_t h_frac = ((r.humidity_q22_10 & 0x3FF) * 100) / 1024;
        uart1_write_int((int32_t)h_whole);
        uart1_write_char('.');
        if (h_frac < 10) uart1_write_char('0');
        uart1_write_int((int32_t)h_frac);

        uart1_write_str("\r\n");

        delay_ms(1000);
        millis += 1000;
        (void)ms_ticks;
    }
}