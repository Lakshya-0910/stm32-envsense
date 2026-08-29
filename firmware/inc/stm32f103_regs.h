/*
 * stm32f103_regs.h
 *
 * Minimal, hand-written register map for the STM32F103C8T6 ("Blue Pill").
 * This project intentionally avoids ST's HAL/CMSIS libraries so every
 * register access is visible and explained -- good for learning, and good
 * for showing an interviewer you understand what's happening under the hood.
 *
 * Only the peripherals this project touches are defined:
 *   - RCC   (clock control)
 *   - GPIOA / GPIOB (pin config)
 *   - I2C1  (talks to the BME280 sensor)
 *   - USART1 (serial output to your PC for logging)
 *   - SysTick (simple delay timer)
 *
 * Reference: RM0008 STM32F1 Reference Manual (register addresses/bits).
 */

#ifndef STM32F103_REGS_H
#define STM32F103_REGS_H

#include <stdint.h>

#define PERIPH_BASE        0x40000000UL
#define APB1PERIPH_BASE     (PERIPH_BASE + 0x00000000UL)
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE       (PERIPH_BASE + 0x00020000UL)

/* ---- RCC (Reset and Clock Control) ---- */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
} RCC_TypeDef;
#define RCC ((RCC_TypeDef *)(AHBPERIPH_BASE + 0x1000UL))

#define RCC_APB2ENR_IOPAEN   (1UL << 2)
#define RCC_APB2ENR_IOPBEN   (1UL << 3)
#define RCC_APB2ENR_USART1EN (1UL << 14)
#define RCC_APB2ENR_AFIOEN   (1UL << 0)
#define RCC_APB1ENR_I2C1EN   (1UL << 21)

/* ---- GPIO ---- */
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;
#define GPIOA ((GPIO_TypeDef *)(APB2PERIPH_BASE + 0x0800UL))
#define GPIOB ((GPIO_TypeDef *)(APB2PERIPH_BASE + 0x0C00UL))
#define GPIOC ((GPIO_TypeDef *)(APB2PERIPH_BASE + 0x1000UL))

/* ---- I2C1 (sensor bus, PB6=SCL, PB7=SDA) ---- */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_TypeDef;
#define I2C1 ((I2C_TypeDef *)(APB1PERIPH_BASE + 0x5400UL))

#define I2C_CR1_PE      (1UL << 0)
#define I2C_CR1_START   (1UL << 8)
#define I2C_CR1_STOP    (1UL << 9)
#define I2C_CR1_ACK     (1UL << 10)
#define I2C_SR1_SB      (1UL << 0)
#define I2C_SR1_ADDR    (1UL << 1)
#define I2C_SR1_BTF     (1UL << 2)
#define I2C_SR1_RXNE    (1UL << 6)
#define I2C_SR1_TXE     (1UL << 7)
#define I2C_SR1_AF      (1UL << 10)

/* ---- USART1 (PA9=TX, PA10=RX, 115200 8N1 to your PC) ---- */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;
#define USART1 ((USART_TypeDef *)(APB2PERIPH_BASE + 0x3800UL))

#define USART_SR_TXE   (1UL << 7)
#define USART_SR_TC    (1UL << 6)
#define USART_CR1_UE   (1UL << 13)
#define USART_CR1_TE   (1UL << 3)
#define USART_CR1_RE   (1UL << 2)

/* ---- FLASH interface (wait states needed above 24MHz) ---- */
typedef struct {
    volatile uint32_t ACR;
} FLASH_TypeDef;
#define FLASH ((FLASH_TypeDef *)(AHBPERIPH_BASE + 0x2000UL))
#define FLASH_ACR_LATENCY_2 (0x2UL)

/* ---- RCC bits for HSE/PLL clock setup (72MHz system clock) ---- */
#define RCC_CR_HSEON      (1UL << 16)
#define RCC_CR_HSERDY     (1UL << 17)
#define RCC_CR_PLLON      (1UL << 24)
#define RCC_CR_PLLRDY     (1UL << 25)
#define RCC_CFGR_PLLSRC_HSE (1UL << 16)
#define RCC_CFGR_PLLMUL9   (0x7UL << 18) /* HSE(8MHz) x9 = 72MHz */
#define RCC_CFGR_PPRE1_DIV2 (0x4UL << 8) /* APB1 = SYSCLK/2 = 36MHz (APB1 max is 36MHz, must divide) */
#define RCC_CFGR_SW_PLL    (0x2UL << 0)
#define RCC_CFGR_SWS_PLL   (0x2UL << 2)

/* ---- SysTick (core timer, used for millisecond delays) ---- */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;
#define SysTick ((SysTick_TypeDef *)(0xE000E010UL))

#endif /* STM32F103_REGS_H */