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
#include "led.h"
#include "led_control.c"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

miLed led1 = {
    .mode = TOGGLE,    
    .n_led = LED_1,    
    .n_ciclos = 10,     
    .periodo = 500 ,    
};

miLed led2 = {
	.mode = ON,
	.n_led = LED_2,
	.n_ciclos = 10,
	.periodo = 500,
};

miLed led3 = {
	.mode = TOGGLE,
	.n_led = LED_3,
	.n_ciclos = 10,
	.periodo = 500,
};
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	LedsInit();
	controlLed(&led3);
}
/*==================[end of file]============================================*/