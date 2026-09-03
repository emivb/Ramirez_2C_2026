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
#include "led.h"
#include "lcditse0803.h"
#include "switch.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/
#define mostrar_delay 100
#define medir_delay 1000
#define teclas_delay 50
/*==================[internal data definition]===============================*/
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
void apagartodo(){
    LedsOffAll();
    LcdItsE0803Off();
    printf("Todo apagado")
}
void Leds(distancia)
{
    switch (distancia)
    {
        
    case >10 & <20:
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
        break;
    case >20 & <30:
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    case >30:
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);

    default:
        LedsOffAll();
        break;
    }
    
}
void toggle(bool *al)
{
    if (*al == true)
    {
        *al = false
    }
    else
    {
        *al= true
    }
    

}

static void Mostrar(void *pvParameter){
    while(true){
        if (ON)
        {
            Leds(distancia);
            if (HOLD== false)
            {
              LcdItsE0803Write(distancia);
            }

            
        }
        else
        {
            apagartodo();
        }
        
        vTaskDelay(mostrar_delay / portTICK_PERIOD_MS);
    }
}

static void Teclas(void *pvParameter){
    while(true){
        uint8_t teclas;
        teclas  = SwitchesRead();
        if (teclas == SWITCH_1)
        {
            toggle(&ON);
        }
        if(teclas == SWITCH_2)
        {
            toggle(&HOLD);
        }
        
        vTaskDelay(teclas_delay / portTICK_PERIOD_MS);
    }
}

static void Medir(void *pvParameter){
    while(true){
        uint8_t teclas;
        if (ON)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        
        
        vTaskDelay(medir_delay / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Deinit();

    uint16_t num = 0;
    LcdItsE0803Write(num);
    bool ON = true;
    bool HOLD = false;

    xTaskCreate(Mostrar, "LED_1", 512, NULL, 5, Mostrar_task_handle); //(5 prioridad mayor 0 mas baja, forma de acceder a la tarea)
    xTaskCreate(Teclas, "LED_2", 512, NULL, 5, Teclas_task_handle);
    xTaskCreate(Medir, "LED_3", 512, NULL, 5, Medir_task_handle);
}
