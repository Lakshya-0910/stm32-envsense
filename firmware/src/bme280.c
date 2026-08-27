/*
 * bme280.c - driver for the Bosch BME280 temperature/humidity/pressure
 * sensor over I2C. The compensation math below follows Bosch's published
 * datasheet formulas (integer fixed-point version) so we don't need
 * floating point on this little Cortex-M3.
 */

#include "bme280.h"
#include "i2c.h"

#define REG_CHIP_ID     0xD0
#define REG_RESET       0xE0
#define REG_CTRL_HUM    0xF2
#define REG_STATUS      0xF3
#define REG_CTRL_MEAS   0xF4
#define REG_CONFIG      0xF5
#define REG_PRESS_MSB   0xF7
#define REG_CALIB_00    0x88
#define REG_CALIB_26    0xE1

#define CHIP_ID_EXPECTED 0x60

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4, dig_H5;
    int8_t   dig_H6;
} bme280_calib_t;

static bme280_calib_t calib;
static int32_t t_fine;

static uint16_t u16le(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static int16_t  s16le(const uint8_t *p) { return (int16_t)(p[0] | (p[1] << 8)); }

static void read_calibration(void) {
    uint8_t b1[26];
    uint8_t b2[7];

    i2c1_read_regs(BME280_I2C_ADDR, REG_CALIB_00, b1, 26);
    calib.dig_T1 = u16le(&b1[0]);
    calib.dig_T2 = s16le(&b1[2]);
    calib.dig_T3 = s16le(&b1[4]);
    calib.dig_P1 = u16le(&b1[6]);
    calib.dig_P2 = s16le(&b1[8]);
    calib.dig_P3 = s16le(&b1[10]);
    calib.dig_P4 = s16le(&b1[12]);
    calib.dig_P5 = s16le(&b1[14]);
    calib.dig_P6 = s16le(&b1[16]);
    calib.dig_P7 = s16le(&b1[18]);
    calib.dig_P8 = s16le(&b1[20]);
    calib.dig_P9 = s16le(&b1[22]);
    calib.dig_H1 = b1[25];

    i2c1_read_regs(BME280_I2C_ADDR, REG_CALIB_26, b2, 7);
    calib.dig_H2 = (int16_t)(b2[0] | (b2[1] << 8));
    calib.dig_H3 = b2[2];
    calib.dig_H4 = (int16_t)((b2[3] << 4) | (b2[4] & 0x0F));
    calib.dig_H5 = (int16_t)((b2[5] << 4) | (b2[4] >> 4));
    calib.dig_H6 = (int8_t)b2[6];
}