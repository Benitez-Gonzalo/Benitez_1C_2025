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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
#define BAUD_RATE 115200 
#define CONFIG_READING_PERIOD 20000
#define BUFFER_SIZE 231
#define CONFIG_ECG_PERIOD 20000
/*==================[internal data definition]===============================*/
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
uint16_t potValue = 0;
TaskHandle_t readValues = NULL;
TaskHandle_t sendValues = NULL;
uint8_t auxIndex = 0;
/*==================[internal functions declaration]=========================*/

void read(){
	vTaskNotifyGiveFromISR(readValues,pdFALSE);
}

void send(){
	vTaskNotifyGiveFromISR(sendValues,pdFALSE);
}

static void readValuesTask (void *pvParameter){

	while(true){
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		AnalogInputReadSingle(CH1,&potValue);
		UartSendString(UART_PC,">ad:");
		UartSendString(UART_PC, (char*)UartItoa(potValue,10));
		UartSendString(UART_PC,"\r\n");
	}

}

static void sendValuesTask (void *pvParameter){

	while (true){
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		AnalogOutputWrite(ecg[auxIndex]); 
		auxIndex = auxIndex + 1;
		if (auxIndex == BUFFER_SIZE){auxIndex=0;};
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

	timer_config_t timer_for_reading = {
		.timer = TIMER_A,
		.period = CONFIG_READING_PERIOD,
		.func_p = read,
		.param_p = NULL,
	};
	TimerInit(&timer_for_reading);

	timer_config_t timer_for_sending = {
		.timer = TIMER_B,
		.period = CONFIG_ECG_PERIOD,
		.func_p = send,
		.param_p = NULL,
	};
	TimerInit(&timer_for_sending);

	analog_input_config_t potInput = {
		.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE,	/*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
	};
	AnalogInputInit(&potInput);
	
	AnalogOutputInit();
	xTaskCreate(&readValuesTask,"readValueTask",2048,NULL,5,&readValues);
	xTaskCreate(&sendValuesTask,"sendValuesTask",2048,NULL,5,&sendValues);
	TimerStart(timer_for_reading.timer);
	TimerStart(timer_for_sending.timer);

}
/*==================[end of file]============================================*/