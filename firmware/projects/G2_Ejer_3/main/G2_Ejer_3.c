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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define mostrar_delay 100
#define medir_delay 1000
#define teclas_delay 200
void Leds(uint16_t distancia); //Le voy diciendo como va a ser la función
void Funcomunicacion(void* param);

/*==================[internal data definition]===============================*/
TaskHandle_t Mostrar_task_handle = NULL;
TaskHandle_t Medir_task_handle = NULL;
bool prender = true;
bool mantener = false;
uint16_t distancia = 0;
uint16_t aux=0;


serial_config_t my_uart = {
    .port      = UART_PC,
    .baud_rate = 115200,
    .func_p    = Funcomunicacion,   // distinto de UART_NO_INT -> UartInit arma la tarea de eventos
    .param_p   = NULL
};


/*==================[internal functions declaration]=========================*/


void apagartodo(){
    LedsOffAll();
    LcdItsE0803Off();
}
void Leds(uint16_t dis)
{
    if (dis>=10 && distancia <20)
    {
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
        
    }
    if (dis >=20 && distancia <30)
    {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);

    }
    if(dis>=30)
    {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
    if(dis<10)
    {
        LedsOffAll();
    }
    
}
void toggle(bool *al)
{
    if (*al == true)
    {
        *al = false;
    }
    else
    {
        *al= true;
    }

}
void Funcomunicacion(void* param){
    uint8_t caracter;
    UartReadByte(UART_PC, &caracter);              // acá se busca el dato que avisó la interrupción
	if (caracter == 'O' || caracter == 'o')
	{
		toggle(&prender);
		UartSendString(UART_PC, "Se apreto la o");  // base 10 -> "1024"
		UartSendString(UART_PC, "\r\n");

	}
	if (caracter == 'H' || caracter == 'h')
	{
		toggle(&mantener);
		UartSendString(UART_PC, "Se apreto la H");  // base 10 -> "1024"
		UartSendString(UART_PC, "\r\n");
	}
}

static void Mostrar_task(void *pvParameter){
    while(true){
        if (prender)
        {
            Leds(distancia);
            if (mantener== false)
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
void Tecla1(void *pvParameter){
    toggle(&prender);
}
void Tecla2(void *pvParameter){
    toggle(&mantener);
}


static void Medir_task(void *pvParameter){
    while(true){
        if (prender)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        if(mantener == false)
        {
            aux = distancia;
        }
        UartSendString(UART_PC, "\nLa");
        UartSendString(UART_PC, (char *) UartItoa(aux, 10));  // base 10 -> "1024"
		UartSendString(UART_PC, "\r\n");
        
        vTaskDelay(medir_delay / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Init(GPIO_3,GPIO_2);
    SwitchActivInt(SWITCH_1,&Tecla1,NULL);
    SwitchActivInt(SWITCH_2,&Tecla2,NULL);
	UartInit(&my_uart);

    xTaskCreate(Mostrar_task, "LED_1", 512, NULL, 5, &Mostrar_task_handle); //(5 prioridad mayor 0 mas baja, forma de acceder a la tarea)
    xTaskCreate(Medir_task, "LED_3", 512, NULL, 5, &Medir_task_handle);
}
