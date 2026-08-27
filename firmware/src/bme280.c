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

uint8_t bme280_init(void) {
    uint8_t id = i2c1_read_reg(BME280_I2C_ADDR, REG_CHIP_ID);
    if (id != CHIP_ID_EXPECTED) {
        return 0;
    }

    read_calibration();

    /* humidity oversampling x1 (must be written before ctrl_meas) */
    i2c1_write_reg(BME280_I2C_ADDR, REG_CTRL_HUM, 0x01);
    /* temp x1, pressure x1, forced mode (sleeps between reads, low power) */
    i2c1_write_reg(BME280_I2C_ADDR, REG_CTRL_MEAS, 0x25);
    /* standby/filter defaults are fine for this use case */
    i2c1_write_reg(BME280_I2C_ADDR, REG_CONFIG, 0x00);

    return 1;
}

/* --- Bosch reference compensation formulas (integer fixed-point) --- */

static int32_t compensate_temperature(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12)
            * ((int32_t)calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8; /* result in 0.01 C */
    return T;
}

static uint32_t compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;
    if (var1 == 0) {
        return 0; /* avoid divide-by-zero */
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    return (uint32_t)(p >> 8); /* Q24.8 fixed-point Pa */
}

static uint32_t compensate_humidity(int32_t adc_H) {
    int32_t v_x1;
    v_x1 = (t_fine - (int32_t)76800);
    v_x1 = (((((adc_H << 14) - (((int32_t)calib.dig_H4) << 20) - (((int32_t)calib.dig_H5) * v_x1))
             + (int32_t)16384) >> 15)
            * (((((((v_x1 * (int32_t)calib.dig_H6) >> 10)
                   * (((v_x1 * (int32_t)calib.dig_H3) >> 11) + (int32_t)32768)) >> 10)
                 + (int32_t)2097152) * (int32_t)calib.dig_H2 + 8192) >> 14));
    v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * (int32_t)calib.dig_H1) >> 4));
    v_x1 = (v_x1 < 0) ? 0 : v_x1;
    v_x1 = (v_x1 > 419430400) ? 419430400 : v_x1;
    return (uint32_t)(v_x1 >> 12); /* Q22.10 fixed-point %RH */
}

void bme280_read(bme280_reading_t *out) {
    uint8_t raw[8];

    /* re-trigger a forced-mode conversion */
    i2c1_write_reg(BME280_I2C_ADDR, REG_CTRL_MEAS, 0x25);

    /* naive fixed delay loop instead of polling the status register --
       fine for x1 oversampling (~10ms max conversion time) */
    for (volatile uint32_t i = 0; i < 200000; i++) { }

    i2c1_read_regs(BME280_I2C_ADDR, REG_PRESS_MSB, raw, 8);

    int32_t adc_P = (int32_t)((raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4));
    int32_t adc_T = (int32_t)((raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4));
    int32_t adc_H = (int32_t)((raw[6] << 8) | raw[7]);

    out->temperature_centi_c = compensate_temperature(adc_T);
    out->pressure_pa_q24_8 = compensate_pressure(adc_P);
    out->humidity_q22_10 = compensate_humidity(adc_H);
}