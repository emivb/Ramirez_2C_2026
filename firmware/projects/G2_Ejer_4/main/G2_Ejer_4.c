/**
 * @file G2_Ejer_1.c
 * @brief Conversión Analógica-Digital (ADC), Digital-Analógica (DAC) de señal ECG y transmisión UART mediante FreeRTOS y Timers.
 *
 * @mainpage G2_Ejer_1
 *
 * @section genDesc Descripción General
 *
 * Este proyecto realiza la lectura de una entrada analógica mediante el ADC del microcontrolador 
 * muestreado periódicamente por el Timer A, enviando los valores leídos a la PC por UART. 
 * A su vez, utiliza el Timer B para enviar digitalmente una señal de ECG almacenada en un buffer
 * hacia la salida analógica (DAC).
 * 
 * @section changelog Historial de Cambios
 *
 * |    Fecha   | Descripción                                    |
 * |:----------:|:-----------------------------------------------|
 * | 17/09/2026 | Creación de documento                          |
 * | 17/09/2026 | Funcionamiento                                 |
 * | 01/10/2026 | Documentación formato doxygen                  |
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
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
/**
 * @def BUFFER_SIZE
 * @brief Tamaño del buffer de muestras de la señal ECG.
 */
#define BUFFER_SIZE 231

/**
 * @brief Función de callback invocada al recibir datos por la interfaz UART.
 * @param param Puntero a parámetros (no utilizado).
 */
void Funcomunicacion(void* param);

/*==================[internal data definition]===============================*/
/** @brief Handle para la tarea encargada de la conversión A/D y envío UART. */
TaskHandle_t ConvertirAD_task_handle = NULL;

/** @brief Handle para la tarea encargada de la conversión D/A (salida ECG). */
TaskHandle_t ConvertirDA_task_handle = NULL;

/** @brief Handle para la tarea principal de la aplicación. */
TaskHandle_t main_task_handle = NULL;

/** @brief Variable global que almacena el valor leído del canal analógico ADC. */
uint16_t valor = 0;

/** @brief Índice del buffer `ecg` para recorrer las muestras del electrocardiograma. */
uint16_t indice = 0;

/** @brief Valor actual del buffer enviado a la salida analógica DAC. */
uint16_t escribir = 0;

/**
 * @brief Buffer digital con la forma de onda de un electrocardiograma (ECG).
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/** @brief Configuración del puerto serie UART para comunicación con la PC. */
serial_config_t my_uart = {
    .port      = UART_PC,
    .baud_rate = 115200,
    .func_p    = Funcomunicacion,
    .param_p   = NULL
};

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función de servicio de interrupción (ISR) del Timer A. 
 *        Notifica a la tarea de conversión A/D.
 * @param param Puntero a parámetros (no utilizado).
 */
void FuncTimerA(void* param);

/**
 * @brief Función de servicio de interrupción (ISR) del Timer B.
 *        Notifica a la tarea de conversión D/A.
 * @param param Puntero a parámetros (no utilizado).
 */
void FuncTimerB(void* param);

static void ConvertirAD_task(void *pvParameter);
static void ConvertirDA_task(void *pvParameter);

/*==================[internal functions definition]==========================*/
void Funcomunicacion(void* param){
    // Callback UART vacía por ahora
}

void FuncTimerA(void* param){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(ConvertirAD_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void FuncTimerB(void* param){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(ConvertirDA_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void ConvertirAD_task(void *pvParameter){
    while(true){        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &valor);
        UartSendString(UART_PC, (char *) UartItoa(valor, 10));
        UartSendString(UART_PC, "\r\n");
    }
}

static void ConvertirDA_task(void *pvParameter){
    while(true){     
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(indice >= 212)
        {
            indice = 0;
        }   
        escribir = ecg[indice];
        AnalogOutputWrite(escribir);
        indice++;
    }
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación. Inicializa perifericos, timers y crea las tareas de FreeRTOS.
 */
void app_main(void){
    // 1. Configuración de entrada analógica usando la estructura de la librería
    analog_input_config_t config_adc = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0
    };
    AnalogInputInit(&config_adc);
    AnalogOutputInit();

    // 2. Configuración e inicialización de UART
    UartInit(&my_uart);

    // 3. Configuración del Timer
    timer_config_t timerAD = {
        .timer = TIMER_A,
        .period = 1000,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timerAD);
    
    timer_config_t timerDA = {
        .timer = TIMER_B,
        .period = 4000,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timerDA);

    // 4. Creación de la tarea de FreeRTOS
    xTaskCreate(ConvertirAD_task, "ConvertirAD", 1024, NULL, 5, &ConvertirAD_task_handle);
    xTaskCreate(ConvertirDA_task, "ConvertirDA", 1024, NULL, 5, &ConvertirDA_task_handle);

    // 5. Inicio del Timer
    TimerStart(timerAD.timer);
    TimerStart(timerDA.timer);
}