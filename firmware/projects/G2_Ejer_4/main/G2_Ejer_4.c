/*! @mainpage G2_Ejer_1
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
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
void Funcomunicacion(void* param);

/*==================[internal data definition]===============================*/
TaskHandle_t ConvertirAD_task_handle = NULL;
uint16_t valor = 0;

serial_config_t my_uart = {
    .port      = UART_PC,
    .baud_rate = 115200,
    .func_p    = Funcomunicacion,
    .param_p   = NULL
};

/*==================[internal functions declaration]=========================*/
void Funcomunicacion(void* param){
    // Callback UART vacía por ahora
}

void FuncTimerA(void* param){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(ConvertirAD_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void ConvertirAD_task(void *pvParameter){
    while(true){        
        // Espera la notificación del Timer
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Lee el canal analógico CH1 y guarda el valor en mV
        AnalogInputReadSingle(CH1, &valor);

        // Envía el resultado formateado por UART
        UartSendString(UART_PC, "\n");
        UartSendString(UART_PC, (char *) UartItoa(valor, 10));
        UartSendString(UART_PC, "\r\n");
    }
}

/*==================[external functions definition]==========================*/
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

    // 2. Configuración e inicialización de UART
    UartInit(&my_uart);

    // 3. Configuración del Timer
    timer_config_t timerAD = {
        .timer = TIMER_A,
        .period = 2000,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timerAD);

    // 4. Creación de la tarea de FreeRTOS
    xTaskCreate(ConvertirAD_task, "ConvertirAD", 1024, NULL, 5, &ConvertirAD_task_handle);

    // 5. Inicio del Timer
    TimerStart(timerAD.timer);
}