/*! @mainpage Midterm exam - Electrónica Programable. 
 *
 * @section genDesc General Description
 *
 * The program is a snow risk sensor and a radiation sensor that tells the user when the values are not safe.
 * It turns on an LED when there is snow risk, high radiation or both. If there is no any risk, it turns on just the green LED.
 * The user can turn ON and OFF the system with the keys provided in the PCB.
 *
 * @section hardConn Hardware Connection
 *
 * |   	DHT11		|   ESP-EDU 	|
 * |:--------------:|:--------------|
 * | 	VCC     	|	3V3     	|
 * | 	DATA	 	| 	GPIO_1		|
 * | 	GND		 	| 	GND 		|
 * 
 * |   	RAD		    |   ESP-EDU 	|
 * |:--------------:|:--------------|
 * | 	DATA	 	| 	GPIO_2		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/06/2025 | Document creation		                         |
 *
 * @author Benitez Gonzalo (gonzalo.benitez@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "dht11.h"
#include <switch.h>
#include "led.h"
/*==================[macros and definitions]=================================*/
#define BAUD_RATE 115200
#define CONFIG_SNOW_MEASURE_PERIOD 1000
#define CONFIG_RADIATION_MEASURE_PERIOD 5000
#define CONFIG_KEYS_PERIOD 1000
#define GPIO_DHT11 GPIO_1
#define RED_LED LED_3
#define YELLOW_LED LED_2
#define GREEN_LED LED_1
#define RADIATION_SENSOR_CHANNEL CH2
/*==================[internal data definition]===============================*/
TaskHandle_t detectSnowRiskTask = NULL;
TaskHandle_t readAmbientRadiationTask = NULL;
TaskHandle_t keysTask = NULL;
uint16_t humidity = 0;
uint16_t temperature = 0;
double radiationValue = 0;
int8_t keys = 0;
bool measurementActive = true;
/*==================[internal functions declaration]=========================*/

void snowRiskCallBack (void){
	vTaskNotifyGiveFromISR(detectSnowRiskTask,pdFALSE);
}

void readRadiationCallBack (void){
	vTaskNotifyGiveFromISR(readAmbientRadiationTask,pdFALSE);
}

void sendMessage(void){
	UartSendString(UART_PC, "Temperatura: ");
    UartSendString(UART_PC, (char*)UartItoa(temperature,10));
    UartSendString(UART_PC, "°C - ");
	UartSendString(UART_PC, "Humedad: ");
	UartSendString(UART_PC,(char*)UartItoa(humidity,10));
	UartSendString(UART_PC, "%\n");
}

/**
 * @brief Tarea encargada de medir el riesgo de nieve
 */
static void snowRiskFunction (void *pvParameter) {

	while(true){
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		if(measurementActive){
			dht11Read(humidity,temperature); //Devuelve los valores en "%" y en "°C"
			sendMessage();
			if(humidity > 85 && temperature > 0 && temperature < 2){
				sendMessage();
				UartSendString(UART_PC,"RIESGO DE NEVADA");
				LedOn(RED_LED);
				LedOff(GREEN_LED);
			}
			else{
				LedOn(GREEN_LED);
				LedOff(RED_LED);
			}
		}
	}
}

/**
 * @brief Tarea encargada de medir la radiación ambiente
 */
static void readRadiationFunction (void *pvParameter) {

	while(true){
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		if(measurementActive){
			AnalogInputReadSingle(RADIATION_SENSOR_CHANNEL,&radiationValue);
			UartSendString(UART_PC,"Radiación");
			UartSendString(UART_PC, (char*)UartItoa(radiationValue,10));
			UartSendString(UART_PC,"mR/h\n");
			if (radiationValue > 1.32){
				LedOn(YELLOW_LED);
				LedOff(GREEN_LED);
				UartSendString(UART_PC,"Radiación");
				UartSendString(UART_PC, (char*)UartItoa(radiationValue,10));
				UartSendString(UART_PC,"mR/h - RADIACIÓN ELEVADA\n");
			}
			else{
				LedOn(GREEN_LED);
				LedOff(YELLOW_LED);
			}
		}
	} 
}

/**
 * @brief Tarea encargada de los botones
 */
static void keysFunction (void *pvParameter){

	while(true){
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		keys = SwitchesRead();
		switch(keys){
			case SWITCH_1:
				measurementActive = true;
			break;

			case SWITCH_2:
				measurementActive = false;
			break;
		}
	}

}


/*==================[external functions definition]==========================*/
void app_main(void){

	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = BAUD_RATE,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&my_uart);
	
	timer_config_t timer_for_snow = {
		.timer = TIMER_A,
		.period = CONFIG_SNOW_MEASURE_PERIOD,
		.func_p = snowRiskFunction,
		.param_p = NULL
	};
	TimerInit(&timer_for_snow);

	timer_config_t timer_for_radiation = {
		.timer = TIMER_B,
		.period = CONFIG_RADIATION_MEASURE_PERIOD,
		.func_p = readRadiationFunction,
		.param_p = NULL
	};
	TimerInit(&timer_for_radiation);

	timer_config_t timer_for_keys = {
		.timer = TIMER_C,
		.period = CONFIG_KEYS_PERIOD,
		.func_p = keysFunction,
		.param_p = NULL,
	};
	TimerInit(&timer_for_keys);

	analog_input_config_t value = {
		.input = RADIATION_SENSOR_CHANNEL,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&value);

	LedsInit();
	SwitchesInit();
	dht11Init(GPIO_DHT11);

	xTaskCreate(&snowRiskFunction, "snowRiskFunction", 2048, NULL, 5, &detectSnowRiskTask);
	xTaskCreate(&readRadiationFunction,"readRadiationFunction",2048, NULL, 5, &readAmbientRadiationTask);
	xTaskCreate(&keysFunction,"keysFunction",2048, NULL, 5, &keysTask);

}
/*==================[end of file]============================================*/