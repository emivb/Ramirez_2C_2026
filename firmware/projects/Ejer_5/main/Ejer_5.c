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
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
void seteado(uint8_t BCD, gpioConf_t config[4]){
	for (size_t i = 0; i < 4; i++)
	{
		uint8_t num =0;
		num = (BCD >>i) & 1;
		if (num==1){
			GPIOOn(config[i].pin);
		}
		else{
			GPIOOff(config[i].pin);
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	gpioConf_t cont[4];
	for (size_t i = 0; i < 4; i++)
	{
		cont[i].pin =(20+i);
		cont[i].dir = 1;
		GPIOInit(cont[i].pin, GPIO_OUTPUT);
	}
	uint8_t num;
	num = 7;
	seteado(num, cont);
}

/*==================[end of file]============================================*/