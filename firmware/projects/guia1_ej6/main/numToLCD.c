#include <stdio.h>
#include <stdint.h>
#include "C:\Repositorios\Benitez_1C_2025\firmware\projects\guia1_ej4\main\convertToBCDArray.c"
#include "C:\Repositorios\Benitez_1C_2025\firmware\projects\guia1_ej5\main\GPIOchange.c"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

/**
 * @fn int8_t numToLCD(uint32_t data, uint8_t digitsNumber, gpioConf_t *gpio_bcd, gpioConf_t *gpio_select)
 * @brief Muestra un número en un display de 7 segmentos.
 *
 * Esta función convierte un número de 32 bits en dígitos BCD y los muestra en un display de 7 segmentos
 * de hasta 3 dígitos. Actualiza los dígitos de manera continua, enviando los datos BCD a través de un 
 * vector de GPIOs y seleccionando cada dígito mediante otro vector de GPIOs. Los datos se latchean en 
 * decodificadores CD4543 con un pulso de 50 ns en los pines de selección. 
 * Reutiliza funciones de ejercicios anteriores: una para convertir el número a BCD (`convertToBcdArray`) y 
 * otra para configurar los GPIOs según un dígito BCD (`setGPIOsFromBCD`).
 *
 * @param[in] data Número de 32 bits a mostrar en el display (máximo 999)
 * @param[in] digitsNumber Cantidad de dígitos a mostrar (máximo 3 según el hardware)
 * @param[in] gpio_bcd Vector de estructuras gpioConf_t para los datos BCD (4 bits: GPIO_20 a GPIO_23)
 * @param[in] gpio_select Vector de estructuras gpioConf_t para la selección de dígitos (GPIO_19, GPIO_18, GPIO_9)
 * @return int8_t 0 si la operación fue exitosa, -1 si hubo un error (parámetros inválidos o fallo en conversión BCD).
 *         El valor de retorno "0" nunca se alcanza debido al bucle infinito.
 */
int8_t numToLCD(uint32_t data, uint8_t digitsNumber, gpioConf_t *gpio_bcd, gpioConf_t *gpio_select) {
    // Validar parámetros
    if (gpio_bcd == NULL || gpio_select == NULL || digitsNumber == 0 || digitsNumber > 3) {
        return -1;
    }

    // Convertir el número a un arreglo de dígitos BCD
    uint8_t bcd_digitsNumber[3];  // Máximo 3 dígitos según el hardware
    if (convertToBcdArray(data, digitsNumber, bcd_digitsNumber) != 0) {
        return -1;
    }

    // Bucle infinito para mostrar los dígitos rápidamente uno tras otro
    while (1) {
        // Iterar sobre cada dígito del display
        for (uint8_t i = 0; i < digitsNumber; i++) {
            // Configurar los datos BCD para el dígito actual
            if (setGPIOsFromBCD(bcd_digitsNumber[i], gpio_bcd) != 0) {
                return -1;
            }

            // Generar un pulso de 50 ns en el pin de selección del dígito
            GPIOState(gpio_select[i].pin, 1);  // Activar el pin (latch)
            esp_rom_delay_us(1);  // Retardo de 1 us (mínimo para garantizar 50 ns)
            GPIOState(gpio_select[i].pin, 0);  // Desactivar el pin

            // Retardo para la multiplexación 
            vTaskDelay(5 / portTICK_PERIOD_MS);  // 5 ms por dígito
        }
    }

    return 0; //Este valor no debe alcanzarse nunca
}