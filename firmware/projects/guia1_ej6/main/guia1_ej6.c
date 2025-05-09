/*! @mainpage Guia 1, ejercicio 6:
 *  
 *
 * @section Mostrar un número en un display de siete segmentos.
 *
 * El programa muestra un número de 32 bits en un display de 7 segmentos de hasta 3 dígitos,
 * utilizando multiplexación para actualizar los dígitos de manera continua. El programa inicializa
 * los GPIOs correspondientes a los datos BCD (D1-D4) y la selección de dígitos (SEL_1-SEL_3), y 
 * luego llama a la función "numToLCD" para mostrar el número.
 *
 *
 * @section hardConn Hardware Connection
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
 * | 09/04/2025 | Document creation		                         |
 *
 * @author Benitez Gonzalo (gonzalo.benitez@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "numToLCD.c"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
gpioConf_t gpio_bcd[4] = {
	{GPIO_20, 1},  // b0 -> GPIO_20, salida
	{GPIO_21, 1},  // b1 -> GPIO_21, salida
	{GPIO_22, 1},  // b2 -> GPIO_22, salida
	{GPIO_23, 1}   // b3 -> GPIO_23, salida
};

gpioConf_t gpio_select[3] = {
	{GPIO_19, 1},  // Dígito 1 -> GPIO_19 (SEL_1)
	{GPIO_18, 1},  // Dígito 2 -> GPIO_18 (SEL_2)
	{GPIO_9,  1}   // Dígito 3 -> GPIO_9  (SEL_3)
};
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	// Inicializar los GPIOs
    for (uint8_t i = 0; i < 4; i++) {
        GPIOInit(gpio_bcd[i].pin, gpio_bcd[i].dir);
    }
    for (uint8_t i = 0; i < 3; i++) {
        GPIOInit(gpio_select[i].pin, gpio_select[i].dir);
    }

    // Número de prueba
    uint32_t number = 345;
    uint8_t digits = 3;

    // Mostrar el número en el display
    int8_t result = numToLCD(number, digits, gpio_bcd, gpio_select);
    if (result != 0) {
        printf("Error al mostrar el número en el display\n");
    }
}
/*==================[end of file]============================================*/