#include <stdio.h>
#include <stdint.h>
#include "gpioConf.c"

int8_t setGPIOsFromBCD(uint8_t bcd_digit, gpioConf_t *gpio_vector) {
    if (gpio_vector == NULL) {
        return -1;
    }

    if (bcd_digit > 9) {
        return -1;
    }

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t bit = (bcd_digit >> i) & 0x01;
        GPIOState(gpio_vector[i].pin, bit);
    }

    return 0;
}