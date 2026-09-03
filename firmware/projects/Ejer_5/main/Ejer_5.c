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
 * |    Peripheral  |   ESP32C6   	|
 * |:--------------:|:--------------|
 * |      D1        |   GPIO_20     |
 * |      D2        |   GPIO_21     |
 * |      D3        |   GPIO_22     |
 * |      D4        |   GPIO_23     |
 * |      +5V       |     +5V       |
 * |      GND       |     GND       |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/08/2026 | Document creation		                         |
 * | 26/08/2026 | Document finalization	                         |
 *
 * @author Emiliano Ramirez (emilianoalanizvb@gmail.com)
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