/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
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
#include "switch.h"


/*==================[macros and definitions]=================================*/



/*==================[internal data definition]===============================*/
typedef enum {
    ON,
	OFF,
	TOGGLE,
} mode;

typedef struct {
	uint8_t mode;  //ON OFF Toggle
	uint8_t n_led;	//Numero de led a modificar
	uint8_t n_ciclos; //Cantidad de ciclos 
	uint16_t periodo;	//Tiempo prendido y tiempo apagado
}my_leds;


/*==================[internal functions declaration]=========================*/
void leds(my_leds *t)
{
	switch(t->mode)
		{
			case ON:
				LedOn(t->n_led);
				break;
			case OFF:
				LedOff(t->n_led);
				break;
			case TOGGLE:
				for (size_t i = 0; i < 2*t->n_ciclos; i++)
				{
					LedToggle(t->n_led);
					vTaskDelay(t->periodo/ portTICK_PERIOD_MS);
					printf("Prueba %u",i);
				}
				
		}
		
	
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	my_leds control;
	control.mode = TOGGLE;
	control.n_led = LED_1;
	control.n_ciclos =5;
	control.periodo =500;
	leds(&control);
	

}
/*==================[end of file]============================================*/