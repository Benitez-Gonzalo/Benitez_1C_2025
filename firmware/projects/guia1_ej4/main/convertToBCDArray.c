#include <stdio.h>
#include <stdint.h>

int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
    if (digits == 0 || bcd_number == NULL){
        return -1; //Error número de dígitos inválido o puntero nulo
    }

    //Variables locales de la función
    uint32_t temp_data = data;
    uint8_t digit_accountant = 0;
    uint8_t local_digits[10]; //"10" es el número máximo de posiciones que puede tener el arreglo
    
    // Caso especial: si data es 0
    if (temp_data == 0) {
        local_digits[0] = 0;
        digit_accountant = 1;
    } else {
        // Extraer dígitos dividiendo entre 10
        while (temp_data > 0) {
            local_digits[digit_accountant] = temp_data % 10;  // Obtener el dígito menos significativo. Por ejemplo, si temp_data = 12345 => temp_data % 10 = 5.
            temp_data /= 10; // Dividir para el siguiente dígito. Ésto elimina el dígito menos significativo. Por ejemplo, en 12345 nos elimina el 5.
            digit_accountant++;
        }
    }

    // Verificar si el número de dígitos es mayor al esperado
    if (digit_accountant > digits) {
        return -1;  // Error: el número tiene más dígitos de los que se pueden almacenar
    }

    // Rellenar el arreglo bcd_number con los dígitos (en orden inverso)
    for (uint8_t i = 0; i < digits; i++) {
        if (i < digit_accountant) {
            bcd_number[digits - 1 - i] = local_digits[i]; //"digits -1 - i" me asegura que los datos se cargan en el vector en el orden correcto.
        } else {
            bcd_number[digits - 1 - i] = 0; // Rellenar con ceros a la izquierda si hay menos dígitos de los esperados
        }
    }

    return 0;  // Éxito

}