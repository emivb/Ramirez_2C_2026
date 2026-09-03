/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 blink.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 06/08/2023 | Document creation		                         |
 * | 06/08/2026 | Document finalization	                         |
 *
 * @author Emiliano Ramirez (emilianoalanizvb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 5000
#define CONFIG_BLINK_2 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit(); //Inicializacion de la salida para Leds
    while(true){
        printf("LED ON\n");
        LedOn(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        printf("LED OFF\n");
        LedOff(LED_1);
        vTaskDelay(CONFIG_BLINK_2 / portTICK_PERIOD_MS);
    }
}
/*==================[end of file]============================================*/
