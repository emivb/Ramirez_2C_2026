/*! @mainpage Proyecto final
 *
 * @section genDesc General Description
 *
 * Estimación de actitud espacial (AHRS) a 100 Hz con el MPU9265/MPU9250 y filtro Madgwick.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32      |
 * |:--------------:|:-------------|
 * |  MPU9265 VCC   |    3.3V      |
 * |  MPU9265 GND   |    GND       |
 * |  MPU9265 SDA   |   GPIO_6     |
 * |  MPU9265 SCL   |   GPIO_7     |
 *
 * @section changelog Changelog
 *
 * |    Date    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/10/2026 | Integración de MPU9265 con calibración y UART  |
 *
 * @author Emiliano Ramirez 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "i2c_mcu.h"
#include "uart_mcu.h"
#include "mpu9250.h"
#include "madgwick.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TIMER_US 10000   /* 100 Hz */
#define MADGWICK_BETA                0.1f

/*==================[internal data declaration]==============================*/
static madgwick_t filter;
static mpu9250_t mpu;
TaskHandle_t Madgwick_task_handle = NULL;

serial_config_t my_uart = {
    .port      = UART_PC,
    .baud_rate = 115200,
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
    float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;
    char buffer[128];

    while (1) {
        /* Esperar la notificación del Timer A cada 10 ms (100 Hz) */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* Intentar leer acelerómetro y giroscopio */
        if (MPU9250ReadAccel(&mpu, &accel) && MPU9250ReadGyro(&mpu, &gyro)) {
            
            /* Intentar leer magnetómetro; si no responde, usar ceros temporalmente */
            if (!MPU9250ReadMag(&mpu, &mag)) {
                mag.x = 0.0f; mag.y = 0.0f; mag.z = 0.0f;
            }

            /* Actualizar filtro Madgwick (se adapta automáticamente a 6-DOF si mag es 0) */
            MadgwickUpdate(&filter, 
                           gyro.x, gyro.y, gyro.z, 
                           accel.x, accel.y, accel.z, 
                           mag.x, mag.y, mag.z);

            /* Obtener ángulos */
            MadgwickGetEulerAngles(&filter, &roll, &pitch, &yaw);

            /* Transmitir por UART */
            snprintf(buffer, sizeof(buffer), "Roll: %6.2f | Pitch: %6.2f | Yaw: %6.2f\r\n", roll, pitch, yaw);
            UartSendString(UART_PC, buffer);
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void) {
    UartInit(&my_uart);
    UartSendString(UART_PC, "--- INICIANDO MPU9265 + MADGWICK ---\r\n");

    I2C_initialize(I2C_MASTER_FREQ_HZ);

    if (!MPU9250Init(&mpu, MPU9250_I2C_ADDR_DEFAULT)) {
        if (!MPU9250Init(&mpu, MPU9250_I2C_ADDR_ALT)) {
            UartSendString(UART_PC, "Error: No se detecto el MPU9265\r\n");
            while (1) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }

    /* Calibración del MPU9265 */
    UartSendString(UART_PC, "Calibrando sensor en reposo...\r\n");
    MPU9250Calibrate(&mpu);
    UartSendString(UART_PC, "Calibracion completada.\r\n");

    MadgwickInit(&filter, 100.0f, MADGWICK_BETA);

    xTaskCreate(Madgwick_task, "Madgwick_task", 8192, NULL, 5, &Madgwick_task_handle);

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