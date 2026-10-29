#ifndef MADGWICK_H
#define MADGWICK_H

/** \addtogroup Drivers_Programable Drivers Programable
 ** @{ */
/** \addtogroup Drivers_Devices Drivers devices
 ** @{ */
/** \addtogroup MADGWICK Madgwick
 ** @{ */

/** @brief Driver para filtro de fusión de sensores IMU/AHRS Madgwick.
 *
 *  Permite estimar la orientación espacial (Roll, Pitch, Yaw) procesando lecturas
 *  de acelerómetro, giroscopio y magnetómetro mediante un algoritmo de descenso de gradiente.
 *
 * @author Sebastian Madgwick / Adaptación
 * 
 * @section changelog
 *
 * |    Date    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 08/10/2026 | Document creation                              |
 *
 */

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief Estructura de control y estado del filtro Madgwick
 */
typedef struct {
    float beta;               /*!< Ganancia del algoritmo (por defecto ~0.1f) */
    float q0;                 /*!< Componente w del cuaternión */
    float q1;                 /*!< Componente x del cuaternión */
    float q2;                 /*!< Componente y del cuaternión */
    float q3;                 /*!< Componente z del cuaternión */
    float invSampleFreq;      /*!< Inverso de la frecuencia de muestreo (1.0f / freq) */
} madgwick_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Inicializa la estructura del filtro Madgwick.
 * 
 * @param filter Puntero a la estructura madgwick_t.
 * @param sample_freq_hz Frecuencia de muestreo en Hz.
 * @param beta Ganancia del filtro.
 */
void MadgwickInit(madgwick_t *filter, float sample_freq_hz, float beta);

/**
 * @brief Actualización del filtro empleando 9 Grados de Libertad (Acelerómetro + Giroscopio + Magnetómetro).
 * 
 * @param filter Puntero a la estructura del filtro.
 * @param gx Giroscopio eje X (rad/s o deg/s).
 * @param gy Giroscopio eje Y.
 * @param gz Giroscopio eje Z.
 * @param ax Acelerómetro eje X.
 * @param ay Acelerómetro eje Y.
 * @param az Acelerómetro eje Z.
 * @param mx Magnetómetro eje X.
 * @param my Magnetómetro eje Y.
 * @param mz Magnetómetro eje Z.
 */
void MadgwickUpdate(madgwick_t *filter, 
                    float gx, float gy, float gz, 
                    float ax, float ay, float az, 
                    float mx, float my, float mz);

/**
 * @brief Actualización del filtro empleando 6 Grados de Libertad (Acelerómetro + Giroscopio).
 * 
 * @param filter Puntero a la estructura del filtro.
 * @param gx Giroscopio eje X.
 * @param gy Giroscopio eje Y.
 * @param gz Giroscopio eje Z.
 * @param ax Acelerómetro eje X.
 * @param ay Acelerómetro eje Y.
 * @param az Acelerómetro eje Z.
 */
void MadgwickUpdateIMU(madgwick_t *filter, 
                       float gx, float gy, float gz, 
                       float ax, float ay, float az);

/**
 * @brief Calcula y devuelve los ángulos de Euler en grados a partir de los cuaterniones.
 * 
 * @param filter Puntero a la estructura del filtro.
 * @param roll Puntero para almacenar el ángulo Roll (grados).
 * @param pitch Puntero para almacenar el ángulo Pitch (grados).
 * @param yaw Puntero para almacenar el ángulo Yaw (grados).
 */
void MadgwickGetEulerAngles(const madgwick_t *filter, float *roll, float *pitch, float *yaw);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif /* #ifndef MADGWICK_H */

/*==================[end of file]============================================*/