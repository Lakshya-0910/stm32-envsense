#ifndef BME280_H
#define BME280_H

#include <stdint.h>

#define BME280_I2C_ADDR 0x76 /* 0x77 if SDO pin is tied high instead of GND */

typedef struct {
    int32_t temperature_centi_c;  /* e.g. 2534 = 25.34 C */
    uint32_t pressure_pa_q24_8;   /* Q24.8 fixed point pascals */
    uint32_t humidity_q22_10;     /* Q22.10 fixed point %RH */
} bme280_reading_t;

/* Returns 1 on success (chip ID matched), 0 on failure (check wiring). */
uint8_t bme280_init(void);

/* Triggers one forced-mode measurement and reads back compensated values. */
void bme280_read(bme280_reading_t *out);

#endif
