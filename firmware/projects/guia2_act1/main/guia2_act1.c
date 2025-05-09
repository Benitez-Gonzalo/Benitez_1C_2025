/*! @mainpage Guía 2, Act1.
 *
 * @section Un programa que mide la distancia en cm con un sensor ultrasónico y lo muestra con LEDs y un LCD.
 *
 * El programa funciona mediante tareas, siendo estas tres: medir, teclas y mostrar.
 * La tarea medir se encarga de devolver la distancia medida en caso de que la medición esté activada.
 * La tarea mostrar se encarga de mostrar la medida a través de un LCD y prendiendo y apagando los LED.
 * La tarea teclas se encarga de modificar el estado de las variables booleanas para activar o desactivar la medición y bloquear o desbloquear el LCD.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    hc-sr04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO        |    GPIO_3     |
 * |    TRIGGER	    |    GPIO_2     |
 * |    +5V         |    +5V        |
 * |    GND 	    |    GND        |
 * 
 * |      LCD       |   ESP32   |
 * |:--------------:|:----------|
 * | 	 D1	 	    |  GPIO_20  |
 * |     D2         |  GPIO_21  |
 * |     D3         |  GPIO_22  |
 * |     D4         |  GPIO_23  |
 * |     SEL_1      |  GPIO_19  |
 * |     SEL_2      |  GPIO_18  |
 * |     SEL_3      |  GPIO_9   |
 * |     +5V        |   +5V     |
 * |     GND        |   GND     |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/04/2025 | Document creation		                         |
 *
 * @author Benitez Gonzalo (gonzalo.benitez@ingenieria.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <hc_sr04.h>
#include <lcditse0803.h>
#include <switch.h>
#include <led.h>
/*==================[macros and definitions]=================================*/
#define CONFIG_MEASURE_PERIOD 1000
#define CONFIG_SHOW_PERIOD 1000
#define CONFIG_KEYS_PERIOD 100
#define GPIO_TRIGGER GPIO_2
#define GPIO_ECHO GPIO_3
#define TEC1 SWITCH_1
#define TEC2 SWITCH_2
/*==================[internal data definition]===============================*/
uint16_t distance = 0;
uint8_t teclas = 0;
bool measurementActive = true;
bool hold = false;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea encargada de la medición de la distancia.
 */
static void measurementTask(void *pvParameter) {
    printf("Entró a la tarea de medición\n");
    while (true) {
        if (measurementActive){
            distance = HcSr04ReadDistanceInCentimeters();
            printf("Distancia medida: %u cm\n", distance);
        }
        else{
            LcdItsE0803Off();
            LedsOffAll();
        }
        vTaskDelay(CONFIG_MEASURE_PERIOD / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Tarea encargada de mostrar la distancia según la medición.
 */
static void showTask(void *pvParameter) {
    printf("Entró a la tarea de mostrar\n");
    while (true){

        //El LCD queda latcheado automáticamente. Es decir, tiene memoria por sí mismo.
        if (!hold){
            LcdItsE0803Write(distance);
        }
        
        if (distance < 10){
            LedsOffAll();
            printf("La distancia es menor a diez cm\n");
        }
        else if (distance > 10 && distance < 20){
            LedOn(LED_1);LedOff(LED_2);LedOff(LED_3);
            printf("La distancia es de entre entre 10 y 20 cm\n");
        }
        else if (distance > 20 && distance < 30){
            LedOn(LED_1);LedOn(LED_2);LedOff(LED_3);
            printf("La distancia es de entre 20 y 30 cm\n");
        }
        else{
            LedOn(LED_1);LedOn(LED_2);LedOn(LED_3);
            printf("La distancia es mayor a 30 cm\n");
        }
        vTaskDelay(CONFIG_SHOW_PERIOD / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Tarea encargada del manejo de las teclas.
 */
static void keysTask(void *pvParameter) {
    printf("Entró a la tarea de las teclas.\n");
    while (true) {
        teclas = SwitchesRead();
        switch(teclas){
            case TEC1:
                measurementActive = !measurementActive;
                if (measurementActive){printf("Se activó la medición.\n");}
                if (!measurementActive){printf("Se desactivó la medición.\n");}
            break;

            case TEC2:
                if(measurementActive){
                    hold = !hold;
                    if (hold){printf("Manteniendo resultado fijo en la pantalla.\n");}
                    if (!hold){printf("Mostrando resultado en tiempo real.\n");}
                }  
            break;
        }
        vTaskDelay(CONFIG_KEYS_PERIOD / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void) {
    printf("Hello world!\n");
    LcdItsE0803Init();
    SwitchesInit();
    LedsInit();
    HcSr04Init(GPIO_ECHO, GPIO_TRIGGER);
    xTaskCreate(measurementTask, "measurementTask", 2048, NULL, 5, NULL);
    xTaskCreate(showTask, "showTask", 2048, NULL, 5, NULL);
    xTaskCreate(keysTask,"keysTask", 2048, NULL, 5, NULL);
}