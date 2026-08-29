#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart1_init(uint32_t baud);
void uart1_write_char(char c);
void uart1_write_str(const char *s);
void uart1_write_int(int32_t val);

#endif
