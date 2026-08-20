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

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
void convertidor(uint16_t dato, uint8_t digitos, int *bcd_num);

/*==================[internal functions declaration]=========================*/

void convertidor(uint16_t dato, uint8_t digitos, int *bcd_num)
{
	int dig;
    for (uint8_t i = 0; i < digitos; i++)
    {
        dig = dato % 10; // Extrae el dígito menos significativo
        dato /= 10;      // Elimina el ultimo digito
		bcd_num[i] = dig;
    }

}
		

/*==================[external functions definition]==========================*/
void app_main(void){

	uint16_t numero = 132; 		// Defino el numero a convertir
    uint8_t cant_digitos = 3;	// Pongo de cuantos digitos es
    int BCD[12]; 			// Array para guardar el bcd

    // Llamada a la función pasando el dato, la cantidad de dígitos y el array
    convertidor(numero, cant_digitos, BCD);

    // Mostrar los resultados
    printf("Numero original: %d\n", numero);
    printf("Digito 0 (Unidades): %d\n", BCD[0]);
    printf("Digito 1 (Decenas):  %d\n", BCD[1]);
    printf("Digito 2 (Centenas): %d\n", BCD[2]);
}
/*==================[end of file]============================================*/