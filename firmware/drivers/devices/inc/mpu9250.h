#ifndef MPU9250_H
#define MPU9250_H

/** \addtogroup Drivers_Programable Drivers Programable
 ** @{ */
/** \addtogroup Drivers_Devices Drivers devices
 ** @{ */
/** \addtogroup MPU9250 MPU9250
 ** @{ */

/** @brief Driver C puro para el sensor MPU9250 / MPU9265 basado en el repositorio ESP32_I2C_MPU9250.
 *
 * @author Adaptación
 * 
 * @section changelog
 *
 * |    Date    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/10/2026 | Adaptación del driver para ESP-EDU e i2c_mcu   |
 *
 */

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>

/*==================[macros]=================================================*/
#define MPU9250_I2C_ADDR_DEFAULT  0x68   /*!< Dirección I2C por defecto (AD0 a GND) */
#define MPU9250_I2C_ADDR_ALT      0x69   /*!< Dirección I2C alternativa (AD0 a VCC) */

/* Escalas del Acelerómetro */
typedef enum {
    GFS_250DPS = 0,
    GFS_500DPS,
    GFS_1000DPS,
    GFS_2000DPS
} mpu9250_gyro_fs_t;

typedef enum {
    AFS_2G = 0,
    AFS_4G,
    AFS_8G,
    AFS_16G
} mpu9250_accel_fs_t;

typedef enum {
    MFS_14BITS = 0, // 0.6 uT per LSB
    MFS_16BITS      // 0.15 uT per LSB
} mpu9250_mag_fs_t;

/*==================[typedef]================================================*/
/**
 * @brief Estructura de vector tridimensional (X, Y, Z).
 */
typedef struct {
    float x;
    float y;
    float z;
} mpu9250_vector_t;

/**
 * @brief Estructura de control y calibración del MPU9250 / MPU9265.
 */
typedef struct {
    uint8_t i2c_addr;
    mpu9250_accel_fs_t accel_fs;
    mpu9250_gyro_fs_t gyro_fs;
    mpu9250_mag_fs_t mag_fs;
    float aRes;
    float gRes;
    float mRes;
    float gyroBias[3];
    float accelBias[3];
    float magCalibration[3];
} mpu9250_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Inicializa y configura el sensor MPU9250/MPU9265 y el magnetómetro AK8963.
 * 
 * @param dev Puntero a la estructura del sensor mpu9250_t.
 * @param i2c_addr Dirección I2C (ej. MPU9250_I2C_ADDR_DEFAULT).
 * @return true Si la inicialización fue correcta.
 */
bool MPU9250Init(mpu9250_t *dev, uint8_t i2c_addr);

/**
 * @brief Calibra el giroscopio y el acelerómetro calculando las variaciones de offset (bias).
 * 
 * @param dev Puntero a la estructura del sensor.
 */
void MPU9250Calibrate(mpu9250_t *dev);

/**
 * @brief Lee los datos del acelerómetro convertidos a g.
 */
bool MPU9250ReadAccel(const mpu9250_t *dev, mpu9250_vector_t *accel);

/**
 * @brief Lee los datos del giroscopio convertidos a deg/s.
 */
bool MPU9250ReadGyro(const mpu9250_t *dev, mpu9250_vector_t *gyro);

/**
 * @brief Lee los datos del magnetómetro AK8963 convertidos a microteslas (uT).
 */
bool MPU9250ReadMag(const mpu9250_t *dev, mpu9250_vector_t *mag);

/**
 * @brief Lee todos los datos (Acelerómetro, Giroscopio y Magnetómetro) simultáneamente.
 */
bool MPU9250ReadALL(const mpu9250_t *dev, mpu9250_vector_t *accel, mpu9250_vector_t *gyro, mpu9250_vector_t *mag);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif /* #ifndef MPU9250_H */

/*==================[end of file]============================================*/