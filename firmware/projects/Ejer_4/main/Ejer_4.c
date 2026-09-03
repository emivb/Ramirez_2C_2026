/*! @mainpage Ejer_4
 *
 * @section genDesc General Description
 *
 * Se le carga un numero de hasta 3 digitos, y lo descompone en bcd almacenandolo en un array
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32C6   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/08/2023 | Document creation		                         |
 * | 23/08/2026 | Document finalization	                         |
 *
 * @author Emiliano Ramirez (emilianoalanizvb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>

/*==================[macros and definitions]=================================*/
void convertidor(uint16_t dato, uint8_t digitos, int *bcd_num); //Le voy diciendo como va a ser la función

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void convertidor(uint16_t dato, uint8_t digitos, int *bcd_num)
{
	int dig;
    for (uint8_t i = 0; i < digitos; i++)
    {
        dig = dato % 10;     // Extrae el dígito menos significativo
        dato /= 10;          // Elimina el ultimo digito
		bcd_num[i] = dig;    // Carga el digito
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	uint16_t numero = 376; 		// Defino el numero a convertir
    uint8_t cant_digitos = 3;	// Pongo de cuantos digitos es
    int BCD[3]; 			    // Array para guardar el bcd
    convertidor(numero, cant_digitos, BCD); // Llamada a la función pasando el dato, la cantidad de dígitos y el array
}

/*==================[end of file]============================================*/