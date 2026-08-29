#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c1_init(void);
uint8_t i2c1_read_reg(uint8_t dev_addr, uint8_t reg_addr);
void i2c1_read_regs(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint16_t len);
void i2c1_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t value);

#endif
