/**
 * @file G2_Ejer_1.c
 * @brief Aplicación de medición de distancia con sensor HC-SR04, display LCD y LEDs mediante FreeRTOS.
 *
 * @mainpage G2_Ejer_1
 *
 * @section genDesc Descripción General
 *
 * Este proyecto realiza la medición de distancia utilizando el sensor ultrasónico HC-SR04,
 * mostrando el resultado en un display LCD (ITSE0803) y representando rangos de distancia 
 * mediante leds en modo barómetro (vúmetro). Además, permite controlar el encendido/apagado 
 * del sistema y la retención del valor mediante switches.
 * 
 * @section changelog Historial de Cambios
 *
 * |    Fecha   | Descripción                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/09/2026 | Creación del documento                         |
 * | 10/09/2026 | Finalizacion del proyecto                      |
 * | 28/09/2026 | Corrección de formato Doxygen                  |
 *
 * @author Emiliano Ramirez
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
/** 
 * @def mostrar_delay
 * @brief Valor en milisegundos para el refresco del display LCD y leds.
 */
#define mostrar_delay 100

/** 
 * @def medir_delay
 * @brief Valor en milisegundos para el tiempo entre cada medición de distancia.
 */
#define medir_delay 1000

/** 
 * @def teclas_delay
 * @brief Valor en milisegundos para el intervalo de lectura del estado de las teclas/switches.
 */
#define teclas_delay 200

/*==================[internal data definition]===============================*/
/** @brief Handle para la tarea encarga de mostrar la información en leds y pantalla. */
TaskHandle_t Mostrar_task_handle = NULL;

/** @brief Handle para la tarea encargada de realizar las mediciones de distancia. */
TaskHandle_t Medir_task_handle = NULL;

/** @brief Handle para la tarea encargada de leer el estado de las teclas. */
TaskHandle_t Teclas_task_handle = NULL;

/** @brief Estado del sistema (true = encendido / encender mediciones, false = apagado). */
bool prender = true;

/** @brief Estado de congelación de pantalla (true = mantener último valor, false = actualizar). */
bool mantener = false;

/** @brief Variable global que almacena el valor actual de la distancia medida en centímetros. */
uint16_t distancia = 0;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Apaga todos los leds y limpia la pantalla LCD.
 */
void apagartodo(void);

/**
 * @brief Utiliza los Leds como si fuese un vúmetro de distancia en centímetros.
 * 
 * Configuración de encendido:
 * - distancia < 10 cm: Todos los leds apagados.
 * - 10 cm <= distancia < 20 cm: LED_1 encendido.
 * - 20 cm <= distancia < 30 cm: LED_1 y LED_2 encendidos.
 * - distancia >= 30 cm: LED_1, LED_2 y LED_3 encendidos.
 *
 * @param dis Distancia medida en centímetros.
 */
void Leds(uint16_t dis);

/**
 * @brief Alterna el valor booleano de una variable apuntada por el parámetro.
 * 
 * @param al Puntero a la variable booleana que se desea conmutar.
 */
void toggle(bool *al);

static void Mostrar_task(void *pvParameter);
static void Teclas_task(void *pvParameter);
static void Medir_task(void *pvParameter);

/*==================[internal functions definition]==========================*/
void apagartodo(void)
{
    LedsOffAll();
    LcdItsE0803Off();
    printf("Todo apagado\n");
}

void Leds(uint16_t dis)
{
    if (dis >= 10 && dis < 20)
    {
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
    else if (dis >= 20 && dis < 30)
    {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    }
    else if (dis >= 30)
    {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
    else
    {
        LedsOffAll();
    }
}

void toggle(bool *al)
{
    
    *al = !(*al);
}

static void Mostrar_task(void *pvParameter)
{
    while(true)
    {
        if (prender)
        {
            Leds(distancia);
            if (!mantener)
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

static void Teclas_task(void *pvParameter)
{
    while(true)
    {
        uint8_t teclas = SwitchesRead();
        if (teclas == SWITCH_1)
        {
            toggle(&prender);
        }
        if (teclas == SWITCH_2)
        {
            toggle(&mantener);
        }
        
        vTaskDelay(teclas_delay / portTICK_PERIOD_MS);
    }
}

static void Medir_task(void *pvParameter)
{
    while(true)
    {
        if (prender)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
            printf("Distancia: %d cm\n", distancia);
        }
        
        vTaskDelay(medir_delay / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación.
 */
void app_main(void)
{
    /* Inicialización de periféricos y sensores */
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    
    /* Creación de tareas de FreeRTOS */
    xTaskCreate(Mostrar_task, "Mostrar_task", 2048, NULL, 5, &Mostrar_task_handle);
    xTaskCreate(Teclas_task,  "Teclas_task",  2048, NULL, 5, &Teclas_task_handle);
    xTaskCreate(Medir_task,   "Medir_task",   2048, NULL, 5, &Medir_task_handle);
}