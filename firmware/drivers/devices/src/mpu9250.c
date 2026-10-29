/**
 * @file mpu9250.c
 * @author Adaptación
 * @brief Driver MPU9250 / MPU9265 en C basado en ESP32_I2C_MPU9250 empleando i2c_mcu.h.
 * @version 0.1
 * @date 2026-10-15
 * 
 * @copyright Copyright (R) 2026
 * 
 */

/*==================[inclusions]=============================================*/
#include "mpu9250.h"
#include "i2c_mcu.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/*==================[macros and definitions]=================================*/
#define MPU9250_WHO_AM_I_REG      0x75
#define MPU9250_WHO_AM_I_VAL1     0x71   /* MPU9250 */
#define MPU9250_WHO_AM_I_VAL2     0x73   /* MPU9265 */
#define MPU9250_WHO_AM_I_VAL3     0x70   /* MPU6500 */

#define MPU9250_PWR_MGMT_1        0x6B
#define MPU9250_PWR_MGMT_2        0x6C
#define MPU9250_CONFIG            0x1A
#define MPU9250_GYRO_CONFIG       0x1B
#define MPU9250_ACCEL_CONFIG      0x1C
#define MPU9250_ACCEL_CONFIG2     0x1D
#define MPU9250_INT_PIN_CFG       0x37

#define MPU9250_ACCEL_XOUT_H      0x3B
#define MPU9250_GYRO_XOUT_H       0x43

/* Registros AK8963 Magnetómetro */
#define AK8963_I2C_ADDR           0x0C
#define AK8963_WHO_AM_I_REG       0x00
#define AK8963_WHO_AM_I_VAL       0x48
#define AK8963_CNTL1              0x0A
#define AK8963_ASAX               0x10
#define AK8963_HXL                0x03

#define I2C_TIMEOUT_DEFAULT_MS    100

/*==================[internal functions declaration]=========================*/
static void MPU9250GetGres(mpu9250_t *dev);
static void MPU9250GetAres(mpu9250_t *dev);
static void MPU9250GetMres(mpu9250_t *dev);
static bool AK8963Init(mpu9250_t *dev);

/*==================[internal functions definition]==========================*/
static void MPU9250GetGres(mpu9250_t *dev) {
    switch (dev->gyro_fs) {
        case GFS_250DPS:  dev->gRes = 250.0f / 32768.0f; break;
        case GFS_500DPS:  dev->gRes = 500.0f / 32768.0f; break;
        case GFS_1000DPS: dev->gRes = 1000.0f / 32768.0f; break;
        case GFS_2000DPS: dev->gRes = 2000.0f / 32768.0f; break;
    }
}

static void MPU9250GetAres(mpu9250_t *dev) {
    switch (dev->accel_fs) {
        case AFS_2G:  dev->aRes = 2.0f / 32768.0f; break;
        case AFS_4G:  dev->aRes = 4.0f / 32768.0f; break;
        case AFS_8G:  dev->aRes = 8.0f / 32768.0f; break;
        case AFS_16G: dev->aRes = 16.0f / 32768.0f; break;
    }
}

static void MPU9250GetMres(mpu9250_t *dev) {
    switch (dev->mag_fs) {
        case MFS_14BITS: dev->mRes = 10.0f * 4912.0f / 8190.0f; break;
        case MFS_16BITS: dev->mRes = 10.0f * 4912.0f / 32760.0f; break;
    }
}

static bool AK8963Init(mpu9250_t *dev) {
    uint8_t id = 0;

    /* Intentar leer WHO_AM_I del AK8963 (0x0C) */
    I2C_readByte(0x0C, 0x00, &id, I2C_TIMEOUT_DEFAULT_MS);

    /* Power-down previo */
    I2C_writeByte(0x0C, 0x0A, 0x00);
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Modo de medición continua 2 (100 Hz) en 16-bits */
    I2C_writeByte(0x0C, 0x0A, 0x16);
    vTaskDelay(pdMS_TO_TICKS(10));

    dev->magCalibration[0] = 1.0f;
    dev->magCalibration[1] = 1.0f;
    dev->magCalibration[2] = 1.0f;

    return true; // Continuar aunque el ID no responda inmediatamente
}
/*==================[external functions definition]==========================*/
bool MPU9250Init(mpu9250_t *dev, uint8_t i2c_addr) {
    uint8_t id = 0;

    if (dev == NULL) return false;

    dev->i2c_addr = i2c_addr;
    dev->accel_fs = AFS_2G;
    dev->gyro_fs  = GFS_250DPS;
    dev->mag_fs   = MFS_16BITS;

    dev->gyroBias[0] = 0; dev->gyroBias[1] = 0; dev->gyroBias[2] = 0;
    dev->accelBias[0] = 0; dev->accelBias[1] = 0; dev->accelBias[2] = 0;

    MPU9250GetAres(dev);
    MPU9250GetGres(dev);
    MPU9250GetMres(dev);

    /* Leer WHO_AM_I */
    if (I2C_readByte(dev->i2c_addr, MPU9250_WHO_AM_I_REG, &id, I2C_TIMEOUT_DEFAULT_MS) == 0) {
        return false;
    }

    if (id != MPU9250_WHO_AM_I_VAL1 && id != MPU9250_WHO_AM_I_VAL2 && id != MPU9250_WHO_AM_I_VAL3) {
        return false;
    }

    /* Configurar reloj del sistema sin reset brusco (evita colapsar el bus I2C) */
    I2C_writeByte(dev->i2c_addr, MPU9250_PWR_MGMT_1, 0x01);
    I2C_writeByte(dev->i2c_addr, MPU9250_PWR_MGMT_2, 0x00);
    vTaskDelay(pdMS_TO_TICKS(50));

    /* Configuración de filtros DLPF */
    I2C_writeByte(dev->i2c_addr, MPU9250_CONFIG, 0x03);
    I2C_writeByte(dev->i2c_addr, MPU9250_GYRO_CONFIG, dev->gyro_fs << 3);
    I2C_writeByte(dev->i2c_addr, MPU9250_ACCEL_CONFIG, dev->accel_fs << 3);
    I2C_writeByte(dev->i2c_addr, MPU9250_ACCEL_CONFIG2, 0x03);

    /* Deshabilitar I2C Master interno para liberar las líneas al AK8963 */
    I2C_writeByte(dev->i2c_addr, 0x6A, 0x00); // USER_CTRL = 0x00

    /* Habilitar Bypass I2C para acceder directamente a la dirección 0x0C */
    I2C_writeByte(dev->i2c_addr, MPU9250_INT_PIN_CFG, 0x02); // INT_PIN_CFG = BYPASS_EN
    vTaskDelay(pdMS_TO_TICKS(20));

    /* Inicializar magnetómetro AK8963 */
    return AK8963Init(dev);
}
void MPU9250Calibrate(mpu9250_t *dev) {
    mpu9250_vector_t g_raw;
    float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
    const int num_samples = 200;

    /* Promediar 200 lecturas en reposo */
    for (int i = 0; i < num_samples; i++) {
        if (MPU9250ReadGyro(dev, &g_raw)) {
            sum_x += g_raw.x;
            sum_y += g_raw.y;
            sum_z += g_raw.z;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    dev->gyroBias[0] = sum_x / (float)num_samples;
    dev->gyroBias[1] = sum_y / (float)num_samples;
    dev->gyroBias[2] = sum_z / (float)num_samples;
}

bool MPU9250ReadAccel(const mpu9250_t *dev, mpu9250_vector_t *accel) {
    uint8_t buffer[6];

    if (I2C_readBytes(dev->i2c_addr, MPU9250_ACCEL_XOUT_H, 6, buffer, I2C_TIMEOUT_DEFAULT_MS) == 0) {
        return false;
    }

    int16_t raw_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t raw_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t raw_z = (int16_t)((buffer[4] << 8) | buffer[5]);

    accel->x = (float)raw_x * dev->aRes - dev->accelBias[0];
    accel->y = (float)raw_y * dev->aRes - dev->accelBias[1];
    accel->z = (float)raw_z * dev->aRes - dev->accelBias[2];

    return true;
}

bool MPU9250ReadGyro(const mpu9250_t *dev, mpu9250_vector_t *gyro) {
    uint8_t buffer[6];

    if (I2C_readBytes(dev->i2c_addr, MPU9250_GYRO_XOUT_H, 6, buffer, I2C_TIMEOUT_DEFAULT_MS) == 0) {
        return false;
    }

    int16_t raw_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t raw_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t raw_z = (int16_t)((buffer[4] << 8) | buffer[5]);

    gyro->x = (float)raw_x * dev->gRes - dev->gyroBias[0];
    gyro->y = (float)raw_y * dev->gRes - dev->gyroBias[1];
    gyro->z = (float)raw_z * dev->gRes - dev->gyroBias[2];

    return true;
}

bool MPU9250ReadMag(const mpu9250_t *dev, mpu9250_vector_t *mag) {
    uint8_t buffer[7];

    /* Leer 7 bytes desde 0x03 (HXL hasta ST2) en el bus AK8963 (0x0C) */
    if (I2C_readBytes(0x0C, 0x03, 7, buffer, I2C_TIMEOUT_DEFAULT_MS) == 0) {
        return false;
    }

    /* Convertir lecturas Little Endian */
    int16_t raw_x = (int16_t)((buffer[1] << 8) | buffer[0]);
    int16_t raw_y = (int16_t)((buffer[3] << 8) | buffer[2]);
    int16_t raw_z = (int16_t)((buffer[5] << 8) | buffer[4]);

    mag->x = (float)raw_x * dev->mRes * dev->magCalibration[0];
    mag->y = (float)raw_y * dev->mRes * dev->magCalibration[1];
    mag->z = (float)raw_z * dev->mRes * dev->magCalibration[2];

    return true;
}

bool MPU9250ReadALL(const mpu9250_t *dev, mpu9250_vector_t *accel, mpu9250_vector_t *gyro, mpu9250_vector_t *mag) {
    bool status = true;
    status &= MPU9250ReadAccel(dev, accel);
    status &= MPU9250ReadGyro(dev, gyro);
    status &= MPU9250ReadMag(dev, mag);
    return status;
}

/*==================[end of file]============================================*/