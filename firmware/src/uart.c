/*
 * uart.c - USART1 driver (PA9=TX, PA10=RX)
 *
 * This is how the board talks to your PC: plug in a cheap USB-to-serial
 * adapter (e.g. CP2102) to PA9/PA10/GND, open a serial terminal (or the
 * scripts/serial_logger.py script in this repo) at the chosen baud rate,
 * and you'll see one CSV line per sensor reading.
 */

#include "stm32f103_regs.h"
#include "uart.h"

#define APB2_CLOCK 72000000UL /* assumes default 72MHz system clock via HSI+PLL, see main.c */

void uart1_init(uint32_t baud) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_AFIOEN;

    /* PA9 = TX: alt-function push-pull, 50MHz -> CRH bits for pin 9 = 0b1011 */
    GPIOA->CRH &= ~(0xFUL << ((9 - 8) * 4));
    GPIOA->CRH |=  (0xBUL << ((9 - 8) * 4));

    /* PA10 = RX: input floating -> CRH bits for pin 10 = 0b0100 */
    GPIOA->CRH &= ~(0xFUL << ((10 - 8) * 4));
    GPIOA->CRH |=  (0x4UL << ((10 - 8) * 4));

    USART1->BRR = APB2_CLOCK / baud;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart1_write_char(char c) {
    while (!(USART1->SR & USART_SR_TXE)) { }
    USART1->DR = (uint8_t)c;
}

void uart1_write_str(const char *s) {
    while (*s) {
        uart1_write_char(*s++);
    }
}

void uart1_write_int(int32_t val) {
    char buf[12];
    int i = 0;
    uint8_t neg = 0;

    if (val < 0) {
        neg = 1;
        val = -val;
    }
    if (val == 0) {
        buf[i++] = '0';
    }
    while (val > 0) {
        buf[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    if (neg) {
        buf[i++] = '-';
    }
    while (i > 0) {
        uart1_write_char(buf[--i]);
    }
}
