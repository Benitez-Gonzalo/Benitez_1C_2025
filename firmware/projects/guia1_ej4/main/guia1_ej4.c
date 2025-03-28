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
#include <inttypes.h>
#include "convertToBCDArray.c"
/*==================[macros and definitions]=================================*/
uint32_t number = 12345;
uint8_t digits = 6;
uint8_t bcd_array[6];
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	int8_t result = convertToBcdArray(number, digits, bcd_array);

    if (result == 0) {
        printf("Número %" PRIu32 " convertido a BCD:\n", number);
        for (uint8_t i = 0; i < digits; i++) {
            printf("Dígito %d: %u\n", i, bcd_array[i]);
        }
    } else {
        printf("Error al convertir el número a BCD\n");
    }
}
/*==================[end of file]============================================*/