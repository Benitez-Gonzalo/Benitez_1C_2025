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
#include "timer_mcu.h"
#include <hc_sr04.h>
#include <lcditse0803.h>
#include <switch.h>
#include <led.h>
#include <uart_mcu.h>
/*==================[macros and definitions]=================================*/
#define CONFIG_MEASURE_PERIOD 1000000
#define CONFIG_SHOW_PERIOD 1000000
#define CONFIG_KEYS_PERIOD 100
#define GPIO_TRIGGER GPIO_2
#define GPIO_ECHO GPIO_3
/*==================[internal data definition]===============================*/
TaskHandle_t measuringDistance = NULL;
TaskHandle_t showDistance = NULL;
uint16_t distance = 0;
uint8_t teclas = 0;
uint8_t teclaPC = 0;
uint16_t maxDistance = 0;
bool measurementActive = true;
bool hold = false;
bool cm = true;
bool max = false;
/*==================[internal functions declaration]=========================*/

void funcKeyTec1(void){
	measurementActive = !measurementActive;
}

void funcKeyTec2(void){
    hold = !hold;
}

void measureFunc(void){
    vTaskNotifyGiveFromISR(measuringDistance, pdFALSE);
}

void showFunc(void){
    vTaskNotifyGiveFromISR(showDistance,pdFALSE);
}

void showDistanceInInches(uint16_t value){
    value = value/2.54;
    UartSendString(UART_PC, "La distancia es: ");
    UartSendString(UART_PC, (char*)UartItoa(value,10));
    UartSendString(UART_PC, "in\n");
}

void showDistanceInCentimenters(uint16_t value){
    UartSendString(UART_PC, "La distancia es: ");
	UartSendString(UART_PC, (char*)UartItoa(value, 10));
	UartSendString(UART_PC, "cm\n");
}
/**
 * @brief Tarea encargada de la medición de la distancia.
 */
static void measurementTask(void *pvParameter) {
    printf("Entró a la tarea de medición\n");
    while (true) {
		
        if (measurementActive){
            distance = HcSr04ReadDistanceInCentimeters();
            //printf("Distancia medida: %u cm\n", distance);
            if(max){
                if (distance > maxDistance){
                    maxDistance = distance;
                }
            }
        }
        else{
            LcdItsE0803Off();
            LedsOffAll();
            maxDistance = 0;
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
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
            if (max) {
                LcdItsE0803Write(maxDistance);
            } else {
                LcdItsE0803Write(distance);
            }
        }
        
        if (distance < 10){
            LedsOffAll();
            //printf("La distancia es menor a diez cm\n");
        }
        else if (distance > 10 && distance < 20){
            LedOn(LED_1);LedOff(LED_2);LedOff(LED_3);
            //printf("La distancia es de entre entre 10 y 20 cm\n");
        }
        else if (distance > 20 && distance < 30){
            LedOn(LED_1);LedOn(LED_2);LedOff(LED_3);
            //printf("La distancia es de entre 20 y 30 cm\n");
        }
        else{
            LedOn(LED_1);LedOn(LED_2);LedOn(LED_3);
            //printf("La distancia es mayor a 30 cm\n");
        }
        if(!cm){
            showDistanceInInches(distance);
        }
        if (max){
            if(!cm){
                showDistanceInInches(maxDistance);
            }
            showDistanceInCentimenters(maxDistance);
        }
		showDistanceInCentimenters(distance); //Este es el caso por defecto
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void readUartKeys(){
	
	while(true){

		if(UartReadByte(UART_PC,&teclaPC)){
			switch (teclaPC)
			{
			case 'O':
				measurementActive = !measurementActive;
				break;

			case 'H':
				hold = !hold;
				break;

            case 'I':
                cm = !cm;
                break;    
			
            case 'M':
                max = !max;
                break;
			default:
				break;

			}
		}
	}
}


/*==================[external functions definition]==========================*/
void app_main(void) {
    printf("Hello world!\n");

	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = readUartKeys,
		.param_p = NULL,
	};
	UartInit(&my_uart);	

    timer_config_t timer_for_measure = {
        .timer = TIMER_A,
        .period = CONFIG_MEASURE_PERIOD,
        .func_p = measureFunc,
        .param_p = NULL
    };
    TimerInit(&timer_for_measure);

    timer_config_t timer_for_showing = {
        .timer = TIMER_B,
        .period = CONFIG_SHOW_PERIOD,
        .func_p = showFunc,
        .param_p = NULL,
    };
    TimerInit(&timer_for_showing);

    LcdItsE0803Init();
    SwitchesInit();
    LedsInit();
    HcSr04Init(GPIO_ECHO, GPIO_TRIGGER);
    SwitchActivInt(SWITCH_1,funcKeyTec1,NULL);
    SwitchActivInt(SWITCH_2,funcKeyTec2,NULL);
    xTaskCreate(&measurementTask, "measurementTask", 2048, NULL, 5, &measuringDistance);
    xTaskCreate(&showTask, "showTask", 2048, NULL, 5, &showDistance);

    TimerStart(timer_for_measure.timer);
    TimerStart(timer_for_showing.timer);
}