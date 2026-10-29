/*! @mainpage Proyecto Final - Ajuste de Escala y Calibración Gyro (x-IMU3)
 *
 * @section genDesc General Description
 *
 * Estimación de orientación espacial a 100 Hz con el MPU-9265 y filtro Madgwick.
 * Aplica calibración de offset en reposo y conversión de unidades (°/s a rad/s)
 * para corregir la sobrerrotación en x-IMU3 GUI.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32-C6   |
 * |:--------------:|:-------------|
 * |  MPU9265 VCC   |    3.3V      |
 * |  MPU9265 GND   |    GND       |
 * |  MPU9265 SDA   |   GPIO_6     | (con SDA unido a EDA)
 * |  MPU9265 SCL   |   GPIO_7     | (con SCL unido a ECL)
 *
 * @section changelog Changelog
 *
 * |    Date    | Description                                         |
 * |:----------:|:----------------------------------------------------|
 * | 15/10/2026 | Sustracción de gyro bias y conversión a rad/s       |
 *
 * @author Emiliano Ramirez 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "i2c_mcu.h"
#include "uart_mcu.h"
#include "mpu9250.h"
#include "madgwick.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TIMER_US 10000   /* 10000 us = 10 ms (100 Hz) */
#define MADGWICK_BETA                0.04f   /* Beta reducido para suavizar la integración */
#define DEG_TO_RAD                   (3.14159265358979323846f / 180.0f)

/*==================[internal data declaration]==============================*/
static madgwick_t filter;
static mpu9250_t mpu;
TaskHandle_t Madgwick_task_handle = NULL;

serial_config_t my_uart = {
    .port      = UART_PC,
    .baud_rate = 230400,                   /* Baudrate incrementado */
    .func_p    = NULL,
    .param_p   = NULL
};

/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param);
static void Madgwick_task(void *pvParameter);

/*==================[internal functions definition]==========================*/
void FuncTimerA(void* param) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(Madgwick_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void Madgwick_task(void *pvParameter) {
    mpu9250_vector_t accel, gyro, mag;
    char buffer[128];

    while (1) {
        /* Esperar notificación del Timer A (100 Hz) */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* 1. Leer Acelerómetro y Giroscopio */
        if (MPU9250ReadAccel(&mpu, &accel) && MPU9250ReadGyro(&mpu, &gyro)) {
            
            /* 2. Leer Magnetómetro con respaldo en caso de time-out */
            if (!MPU9250ReadMag(&mpu, &mag)) {
                mag.x = 0.0f;
                mag.y = 0.0f;
                mag.z = 0.0f;
            }

            /* 3. Convertir giroscopio de deg/s a rad/s */
            float gx_rad = gyro.x * DEG_TO_RAD;
            float gy_rad = gyro.y * DEG_TO_RAD;
            float gz_rad = gyro.z * DEG_TO_RAD;

            /* Timestamp basado en ticks de FreeRTOS en microsegundos */
            uint64_t timestamp_us = (uint64_t)xTaskGetTickCount() * (1000000 / configTICK_RATE_HZ);

            /* 4. Actualización del filtro Madgwick (9-DOF) en rad/s */
            MadgwickUpdate(&filter, 
                           gx_rad, gy_rad, gz_rad, 
                           accel.x, accel.y, accel.z, 
                           mag.x, mag.y, mag.z);

            /* 5. Trama de Cuaternión para x-IMU3 (MSG_ID = 'Q') */
            snprintf(buffer, sizeof(buffer), "Q,%llu,%.4f,%.4f,%.4f,%.4f\r\n", 
                     timestamp_us, filter.q0, filter.q1, filter.q2, filter.q3);
            UartSendString(UART_PC, buffer);

            /* 6. Trama Inercial para x-IMU3 (MSG_ID = 'I') con giro en deg/s */
            snprintf(buffer, sizeof(buffer), "I,%llu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n", 
                     timestamp_us, gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z);
            UartSendString(UART_PC, buffer);

            /* 7. Trama de Magnetómetro para x-IMU3 (MSG_ID = 'M') */
            snprintf(buffer, sizeof(buffer), "M,%llu,%.4f,%.4f,%.4f\r\n", 
                     timestamp_us, mag.x, mag.y, mag.z);
            UartSendString(UART_PC, buffer);
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void) {
    /* 1. Inicializar comunicación UART PC (230400 baudios) */
    UartInit(&my_uart);

    /* 2. Inicializar bus I2C */
    I2C_initialize(I2C_MASTER_FREQ_HZ);

    /* 3. Inicializar MPU-9265 */
    if (!MPU9250Init(&mpu, MPU9250_I2C_ADDR_DEFAULT)) {
        if (!MPU9250Init(&mpu, MPU9250_I2C_ADDR_ALT)) {
            while (1) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }

    /* 4. Calibrar sensores en reposo (mantiene la placa quieta 1 segundo) */
    MPU9250Calibrate(&mpu);

    /* 5. Inicializar filtro Madgwick */
    MadgwickInit(&filter, 100.0f, MADGWICK_BETA);

    /* 6. Crear tarea del filtro */
    xTaskCreate(Madgwick_task, "Madgwick_task", 8192, NULL, 5, &Madgwick_task_handle);

    /* 7. Configurar Timer A a 100 Hz */
    timer_config_t timerMuestra = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TIMER_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timerMuestra);
    TimerStart(timerMuestra.timer);
}

/*==================[end of file]============================================*/