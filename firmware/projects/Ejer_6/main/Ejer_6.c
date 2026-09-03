/*! @mainpage Ejercicio 6 - Display LCD Multiplexado
 *
 * @section genDesc General Description
 *
 * Este programa convierte un valor numérico entero y lo muestra en un display LCD 
 * de 7 segmentos de 3 dígitos multiplexado a través de decodificadores BCD CD4543.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32C6     |
 * |:--------------:|:--------------|
 * |      D1        |   GPIO_20     |
 * |      D2        |   GPIO_21     |
 * |      D3        |   GPIO_22     |
 * |      D4        |   GPIO_23     |
 * |     SEL_1      |   GPIO_19     |
 * |     SEL_2      |   GPIO_18     |
 * |     SEL_3      |   GPIO_9      |
 * |      +5V       |     +5V       |
 * |      GND       |     GND       |
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 02/09/2026 | Document creation                              |
 * | 03/09/2026 | Document finalization                          |
 *
 * @author Emiliano Ramirez (emilianoalanizvb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/*==================[typedef]================================================*/

/**
 * @struct gpioConf_t
 * @brief Estructura de configuración para un pin GPIO.
 */
typedef struct
{
    gpio_t pin;         /*!< GPIO pin number */
    io_t dir;           /*!< GPIO direction '0' IN;  '1' OUT */
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Muestra un dígito BCD (0-9) en las líneas de datos BCD (D1 a D4).
 * 
 * @details Modifica el estado lógico (ON/OFF) de cada pin de datos según
 *          los bits del número BCD recibido por parámetro.
 * 
 * @param[in] BCD Valor del dígito a mostrar en formato BCD (4 bits).
 * @param[in] config Vector de 4 estructuras gpioConf_t que mapean los pines D1-D4.
 */
void seteado(uint8_t BCD, gpioConf_t config[4]){
    for (size_t i = 0; i < 4; i++)
    {
        uint8_t num = (BCD >> i) & 1;
        if (num == 1){
            GPIOOn(config[i].pin);
        }
        else{
            GPIOOff(config[i].pin);
        }
    }
}

/**
 * @brief Muestra un valor numérico entero en el display LCD seleccionando dígito por dígito.
 * 
 * @details Descompone un número entero en sus dígitos individuales y los envía uno a uno 
 *          al display multiplexado, activando correspondientemente los pines de selección (SEL_1 a SEL_3).
 * 
 * @param[in] dato Número entero de 16 bits a representar en la pantalla.
 * @param[in] digitos Cantidad de dígitos de salida del display a utilizar.
 * @param[in] config Vector de configuración para las líneas de datos BCD (D1 a D4).
 * @param[in] display Vector de configuración para los pines de selección del LCD (SEL_1 a SEL_3).
 * 
 * @note El integrado CD4543 enclava el dato (latch) en los Flip-Flops internos mediante el pulso de selección.
 */
void convertidor(uint16_t dato, uint8_t digitos, gpioConf_t config[4], gpioConf_t display[3]){
    uint8_t dig; // Variable auxiliar que va a contener el dígito
    
    for (uint8_t i = digitos; i > 0; i--) 
    {
        dig = dato % 10;                // Extrae el dígito menos significativo
        dato /= 10;                     // Elimina el último dígito
        GPIOOn(display[i-1].pin);       // Habilita el latch del dígito correspondiente
        seteado(dig, config);           // Saca el dato BCD a las líneas de datos
        GPIOOff(display[i-1].pin);      // Deshabilita el latch reteniendo el dato cargado
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 */
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