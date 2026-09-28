/**
 * @file G2_Ejer_2.c
 * @brief Medición de distancia con HC-SR04, display LCD, LEDs y control por interrupciones de switches.
 *
 * @mainpage G2_Ejer_2
 *
 * @section genDesc Descripción General
 *
 * Este proyecto realiza la medición de distancia utilizando el sensor ultrasónico HC-SR04,
 * mostrando el resultado en un display LCD (ITSE0803) y representando rangos de distancia 
 * mediante leds en modo barómetro (vúmetro). El control de encendido/apagado del sistema 
 * y la retención del valor se gestionan mediante interrupciones asociadas a los switches.
 * 
 * @section changelog Historial de Cambios
 *
 * |    Fecha   | Descripción                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/09/2026 | Creación del documento                         |
 * | 17/09/2026 | Finalización del proyecto                      |
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
 * @brief Valor en milisegundos para la verificación del estado de las teclas.
 */
#define teclas_delay 200

/*==================[internal data definition]===============================*/
/** @brief Handle para la tarea encargada de mostrar la información en leds y pantalla. */
TaskHandle_t Mostrar_task_handle = NULL;

/** @brief Handle para la tarea encargada de realizar las mediciones de distancia. */
TaskHandle_t Medir_task_handle = NULL;

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
 * @brief Utiliza los Leds como vúmetro de distancia en centímetros.
 * 
 * Configuración de encendido:
 * - dis < 10 cm: Todos los leds apagados.
 * - 10 cm <= dis < 20 cm: LED_1 encendido.
 * - 20 cm <= dis < 30 cm: LED_1 y LED_2 encendidos.
 * - dis >= 30 cm: LED_1, LED_2 y LED_3 encendidos.
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

/**
 * @brief Función de servicio de interrupción asignada al SWITCH_1.
 * 
 * @param pvParameter Puntero a parámetros (no utilizado).
 */
void Tecla1(void *pvParameter);

/**
 * @brief Función de servicio de interrupción asignada al SWITCH_2.
 * 
 * @param pvParameter Puntero a parámetros (no utilizado).
 */
void Tecla2(void *pvParameter);

static void Mostrar_task(void *pvParameter);
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
    if (al != NULL)
    {
        *al = !(*al);
    }
}

void Tecla1(void *pvParameter)
{
    toggle(&prender);
}

void Tecla2(void *pvParameter)
{
    toggle(&mantener);
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

    /* Configuración de interrupciones de switches */
    SwitchActivInt(SWITCH_1, &Tecla1, NULL);
    SwitchActivInt(SWITCH_2, &Tecla2, NULL);

    /* Creación de tareas de FreeRTOS */
    xTaskCreate(Mostrar_task, "Mostrar_task", 2048, NULL, 5, &Mostrar_task_handle);
    xTaskCreate(Medir_task,   "Medir_task",   2048, NULL, 5, &Medir_task_handle);
}