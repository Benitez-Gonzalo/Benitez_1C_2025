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
#include "GPIOchange.c"
/*==================[macros and definitions]=================================*/
gpioConf_t gpio_mapping[4] = {
	{GPIO_20, 1},  // b0 -> GPIO_20, salida
	{GPIO_21, 1},  // b1 -> GPIO_21, salida
	{GPIO_22, 1},  // b2 -> GPIO_22, salida
	{GPIO_23, 1}   // b3 -> GPIO_23, salida
};

// Dígito BCD de prueba: 5 (0101 en binario)
uint8_t bcd_digit = 5;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){

	// Inicializar los GPIOs
	for (uint8_t i = 0; i < 4; i++) {
		GPIOInit(gpio_mapping[i].pin, gpio_mapping[i].dir);
	}

	// Configurar los GPIOs según el dígito BCD
    int8_t result = setGPIOsFromBCD(bcd_digit, gpio_mapping);

    if (result == 0) {
        printf("GPIOs configurados según BCD %u\n", bcd_digit);
    } else {
        printf("Error al configurar los GPIOs\n");
    }

}
/*==================[end of file]============================================*/