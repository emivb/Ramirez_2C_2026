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
void convertidor(uint16_t dato, uint8_t digitos, gpioConf_t config[4], gpioConf_t display[3] )
{
	uint8_t dig;
    
    for (uint8_t i = digitos; i > 0; i--)
    {
        dig = dato % 10; // Extrae el dígito menos significativo
        dato /= 10;      // Elimina el ultimo digito
		GPIOOn(display[i-1].pin);
		seteado(dig,config);
		GPIOOff(display[i-1].pin);
    }


}

/*==================[external functions definition]==========================*/
void app_main(void){
	gpioConf_t cont[4];
	gpioConf_t dis[3];
	dis[0].pin = 19;
	dis[1].pin = 18;
	dis[2].pin =9;
	for (size_t i = 0; i < 3; i++)
	{
		cont[i].pin =(20+i);
		cont[i].dir = 1;
		dis[i].dir=1;
		GPIOInit(cont[i].pin, GPIO_OUTPUT);
		GPIOInit(dis[i].pin, GPIO_OUTPUT);
	}
	cont[3].pin =(23);
	cont[3].dir = 1;
	GPIOInit(cont[3].pin, GPIO_OUTPUT);
	
	uint16_t numero = 376; 		// Defino el numero a convertir
    uint8_t cant_digitos = 3;	// Pongo de cuantos digitos es

	convertidor(numero, cant_digitos,cont,dis);

}
/*==================[end of file]============================================*/