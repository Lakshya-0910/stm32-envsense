/*
 * i2c.c - I2C1 driver (PB6=SCL, PB7=SDA)
 *
 * Talks to the BME280 sensor. This is standard-mode I2C (100kHz) using
 * the STM32F1's hardware I2C peripheral in polling mode (no interrupts,
 * kept simple on purpose for a first project).
 */

#include "stm32f103_regs.h"
#include "i2c.h"

#define I2C_TIMEOUT 100000UL

static void i2c1_wait(volatile uint32_t *reg, uint32_t bit) {
    uint32_t timeout = I2C_TIMEOUT;
    while (!(*reg & bit) && timeout--) { }
}

void i2c1_init(void) {
    /* enable clocks: GPIOB, AFIO, I2C1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6/PB7 = alternate function open-drain, 50MHz (required for I2C) */
    GPIOB->CRL &= ~((0xFUL << (6 * 4)) | (0xFUL << (7 * 4)));
    GPIOB->CRL |=  ((0xFUL << (6 * 4)) | (0xFUL << (7 * 4))); /* AF open-drain, 50MHz = 0b1111 */

    I2C1->CR1 &= ~I2C_CR1_PE;

    /* peripheral clock = 36MHz (APB1 max on F103); set CR2 freq field */
    I2C1->CR2 = 36;
    /* standard mode 100kHz: CCR = Fpclk1 / (2 * 100000) */
    I2C1->CCR = 180;
    /* max rise time = (1000ns / (1/Fpclk1)) + 1 */
    I2C1->TRISE = 37;

    I2C1->CR1 |= I2C_CR1_PE;
}

static void i2c1_start(void) {
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_START;
    i2c1_wait(&I2C1->SR1, I2C_SR1_SB);
}

static void i2c1_stop(void) {
    I2C1->CR1 |= I2C_CR1_STOP;
}

static void i2c1_send_addr(uint8_t addr7, uint8_t read) {
    I2C1->DR = (addr7 << 1) | (read ? 1 : 0);
    i2c1_wait(&I2C1->SR1, I2C_SR1_ADDR);
    (void)I2C1->SR1;
    (void)I2C1->SR2; /* clear ADDR by reading SR1 then SR2 */
}

void i2c1_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t value) {
    i2c1_start();
    i2c1_send_addr(dev_addr, 0);
    i2c1_wait(&I2C1->SR1, I2C_SR1_TXE);
    I2C1->DR = reg_addr;
    i2c1_wait(&I2C1->SR1, I2C_SR1_TXE);
    I2C1->DR = value;
    i2c1_wait(&I2C1->SR1, I2C_SR1_BTF);
    i2c1_stop();
}

uint8_t i2c1_read_reg(uint8_t dev_addr, uint8_t reg_addr) {
    uint8_t val;
    i2c1_read_regs(dev_addr, reg_addr, &val, 1);
    return val;
}

void i2c1_read_regs(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint16_t len) {
    /* write register pointer first */
    i2c1_start();
    i2c1_send_addr(dev_addr, 0);
    i2c1_wait(&I2C1->SR1, I2C_SR1_TXE);
    I2C1->DR = reg_addr;
    i2c1_wait(&I2C1->SR1, I2C_SR1_BTF);

    /* repeated start, then read `len` bytes */
    i2c1_start();
    i2c1_send_addr(dev_addr, 1);

    for (uint16_t i = 0; i < len; i++) {
        if (i == len - 1) {
            I2C1->CR1 &= ~I2C_CR1_ACK; /* NACK the last byte */
        } else {
            I2C1->CR1 |= I2C_CR1_ACK;
        }
        i2c1_wait(&I2C1->SR1, I2C_SR1_RXNE);
        buf[i] = (uint8_t)I2C1->DR;
    }
    i2c1_stop();
}
