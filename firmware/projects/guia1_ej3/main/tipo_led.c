/*

Estructura de tipo LED que define el tipo que será llamado por la función "controlLed". El "mode" queda definido con la estructura y no es modificable.

*/

#include "stdint.h"

typedef struct {
    uint8_t mode;     // ON, OFF, TOGGLE
    uint8_t n_led;    // Indica el número de LED a controlar
    uint8_t n_ciclos; // Indica la cantidad de ciclos de encendido/apagado
    uint16_t periodo; // Indica el tiempo de cada ciclo en milisegundos
}miLed;


