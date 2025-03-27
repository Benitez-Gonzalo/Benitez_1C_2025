/*

Función para controlar el encendido y apagado de un LED según un switch case.

*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "tipo_led.c"

#define ON      1
#define OFF     2
#define TOGGLE  3

#define CONFIG_BLINK_PERIOD 100

void controlLed(miLed *my_led) {

    uint8_t retardo = my_led->periodo/CONFIG_BLINK_PERIOD;
    switch(my_led->mode){
        
        case(ON):
            printf("Prendiendo led");
            LedOn(my_led->n_led);
        break;

        case(OFF):
            printf("Apagando led");
            LedOff(my_led->n_led);
        break;

        case(TOGGLE):
            printf("Toggleando led");
            for (int i=0;i<my_led->n_ciclos;i++){
                LedToggle(my_led->n_led);
                for (int j=0;j<retardo;j++){
                    vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
                }
            }
        break;

        default:
            printf("Error");
        break;
    }

}