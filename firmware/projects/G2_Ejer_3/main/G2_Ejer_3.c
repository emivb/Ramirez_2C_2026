/**
 * @file G2_Ejer_3.c
 * @brief Medición de distancia con HC-SR04, display LCD, LEDs, switches por interrupción y puerto serie UART.
 *
 * @mainpage G2_Ejer_3
 *
 * @section genDesc Descripción General
 *
 * Este programa lee una medición de distancia mediante un sensor ultrasónico HC-SR04 y la
 * muestra en un display LCD e indicadores LEDs en modo barómetro (vúmetro). Permite
 * controlar el encendido/apagado y la retención mediante switches (interrupciones)
 * y comandos recibidos por el puerto serie (UART PC). Además, envía la distancia medida por UART.
 * 
 * @section changelog Historial de Cambios
 *
 * |    Fecha   | Descripción                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/09/2026 | Creación de documento                          |
 * | 17/09/2026 | Estado funcional, falta documentar             |
 * | 01/10/2026 | Documentación formato doxygen                  |
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/** 
 * @def mostrar_delay
 * @brief Valor en milisegundos para el refresco del display LCD y LEDs.
 */
#define mostrar_delay 100

/** 
 * @def medir_delay
 * @brief Valor en milisegundos para el intervalo entre cada medición de distancia y envío UART.
 */
#define medir_delay 1000

/**
 * @brief Utiliza los LEDs como vúmetro según la distancia especificada.
 * @param distancia Distancia medida en centímetros.
 */
void Leds(uint16_t distancia);

/**
 * @brief Función de callback invocada al recibir datos por la UART.
 * @param param Puntero a parámetro (no utilizado).
 */
void Funcomunicacion(void* param);

/*==================[internal data definition]===============================*/
/** @brief Handle para la tarea encargada de actualizar la pantalla LCD y los LEDs. */
TaskHandle_t Mostrar_task_handle = NULL;

/** @brief Handle para la tarea encargada de realizar la medición con el sensor HC-SR04 y enviar por UART. */
TaskHandle_t Medir_task_handle = NULL;

/** @brief Estado de encendido/apagado del sistema. */
bool prender = true;

/** @brief Estado de retención/congelamiento de la pantalla y valor medido. */
bool mantener = false;

/** @brief Variable global que almacena la distancia medida en centímetros. */
uint16_t distancia = 0;

/** @brief Variable auxiliar que guarda la última distancia congelada o en tiempo real para transmisión UART. */
uint16_t aux = 0;

/** @brief Configuración de la interfaz serie UART para comunicación con la PC. */
serial_config_t my_uart = {
    .port      = UART_PC,
    .baud_rate = 115200,
    .func_p    = Funcomunicacion,   // distinto de UART_NO_INT -> UartInit arma la tarea de eventos
    .param_p   = NULL
};

/*==================[internal functions declaration]=========================*/
/**
 * @brief Apaga todos los LEDs y deshabilita la pantalla LCD.
 */
void apagartodo(void);

/**
 * @brief Controla el encendido de LEDs (vúmetro) según el rango de distancia recibido.
 * @param dis Distancia en centímetros.
 */
void Leds(uint16_t dis);

/**
 * @brief Conmuta el estado de una variable booleana recibida por referencia.
 * @param al Puntero a la variable booleana a conmutar.
 */
void toggle(bool *al);

/**
 * @brief Función ISR/callback de la tecla 1. Conmuta el estado de encendido (`prender`).
 * @param pvParameter Puntero a parámetros (no utilizado).
 */
void Tecla1(void *pvParameter);

/**
 * @brief Función ISR/callback de la tecla 2. Conmuta el estado de retención (`mantener`).
 * @param pvParameter Puntero a parámetros (no utilizado).
 */
void Tecla2(void *pvParameter);

static void Mostrar_task(void *pvParameter);
static void Medir_task(void *pvParameter);

/*==================[internal functions definition]==========================*/
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
    *al = !(*al);
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
        
        UartSendString(UART_PC, (char *) UartItoa(aux, 10));  
        UartSendString(UART_PC, " cm");
        UartSendString(UART_PC, "\r\n");
        
        vTaskDelay(medir_delay / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal del sistema. Inicializa periféricos, UART e interrupciones y crea las tareas.
 */
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