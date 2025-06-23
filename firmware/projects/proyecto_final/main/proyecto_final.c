
/*! @mainpage P.Electronics System
 *
 * @section genDesc General Description
 *
 * This program measures an unknown resistance (R2) using a voltage divider connected to GPIO0 (CH0) of the ESP32-C6.
 * A known resistance (R1 = 10 kOhm) is connected between GND and GPIO9 for a PWM, and the unknown resistance (R2) is
 * connected between R1 and GND. The voltage is read using the analog_io_mcu driver, and the resistance is
 * calculated and displayed via the serial port. Then, the value is send through Telegram to a bot periodically.
 * This value is used to calculate water conductivity (using the water as R2) and its hardness. If this last value is
 * too high, a filter is deployed into the water sample.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral          |   ESP32-C6   	|
 * |:----------------------:|:-----------------|
 * | 	R1 (10 kOhm) (+)      | 	3.3V			|
 * | 	R1 (10 kOhm) (-)      | 	GPIO0 (CH0)		|
 * | 	R2 (Unknown) (+)      | 	GPIO0 (CH0)		|
 * | 	R2 (Unknown) (-)      | 	GND				|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 * | 12/05/2025 | Updated for ESP32-C6 and ESP-IDF v5.4.0        |
 * | 12/05/2025 | Using analog_io_mcu driver for ADC readings    |
 *
 * @author Gonzalo Benitez (gonzalo.benitez@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include "timer_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "telegram_bot_mcu.h"
#include "servo_sg90.h"
#include "pwm_mcu.h"
/*==================[macros and definitions]=================================*/
#define R1  10000.0 // Ohm (resistencia conocida)
#define CONFIG_MEASURE_PERIOD 1000000
#define CONFIG_TELEGRAM_PERIOD 3000000
#define WIFI_SSID "moto-e(7)-Gonza"
#define WIFI_PASS "gonza235"
#define BOT_TOKEN "7666661023:AAGC_aNd2ElAc5ieqiJxhCb8IVP74LCZG1o"
#define CHAT_ID "7725635002" 
#define UMBRAL_KOHM 5 //Umbral resistivo en kohm
#define CONVERSION_FACTOR 0.145
/*==================[internal data definition]===============================*/
TaskHandle_t measuringResistanceTask = NULL;
TaskHandle_t telegramMessageTask = NULL;
uint16_t voltage = 0;
uint32_t resistance = 0;
uint16_t adc_raw = 0;
float v_adc = 0;
float r_water = 0;
float conductivity = 0;
float water_hardness = 0;
char buffer[100];
char message[128];
extern const uint8_t _binary_telegram_cert_pem_start[];
/*==================[internal functions declaration]=========================*/

void showVoltage(uint16_t value){
    UartSendString(UART_PC, "El voltaje medido es: ");
	UartSendString(UART_PC, (char*)UartItoa(value, 10));
	UartSendString(UART_PC, "mV\n");
}

void showResistance(uint32_t value){
    UartSendString(UART_PC, "La resistencia medida es: ");
	UartSendString(UART_PC, (char*)UartItoa(value, 10));
	UartSendString(UART_PC,"ohm");
    UartSendString(UART_PC,"\r\n");
}

void measuringResistanceCallBack (void){
    vTaskNotifyGiveFromISR(measuringResistanceTask,pdFALSE);
}

void telegramMessageCallBack (void){
    vTaskNotifyGiveFromISR(telegramMessageTask,pdFALSE);
}

/**
 * @brief Tarea encargada de la medición de la resistencia con DC

static void measuringResistanceFunction(void *pvParameter){
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH0,&voltage);
        showVoltage(voltage);

        // Calcular R2 a partir del divisor de voltaje
        float Vout = voltage / 1000.0; // Convertir a voltios
        float Vcc = 3.3; // Voltaje de suministro
        if (Vout > 0 && Vout < Vcc) {
            resistance = (Vout * R1) / (Vcc - Vout);
            showResistance(resistance);
            if(resistance > UMBRAL_KOHM * 1000){ServoMove(SERVO_0,90);}
            else{ServoMove(SERVO_0,0);}
        } else {
            UartSendString(UART_PC, "Error, voltaje fuera de rango para cálculo");
            UartSendString(UART_PC, (char*)UartItoa(Vout, 10));
            UartSendString(UART_PC, "V\n");
        }  
    }
}
*/

/**
 * @brief Tarea encargada de enviar el mensaje vía Telegram
*/
static void telegramMessageFunction(void *pvParameter) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        //snprintf(message, sizeof(message), "Resistencia medida: %lu ohm", resistance);

        snprintf(buffer, sizeof(buffer),"Cwater = %.0f µS/cm\n", r_water);

        //TelegramSendMessage(BOT_TOKEN, CHAT_ID, message, (const char *)_binary_telegram_cert_pem_start);
        TelegramSendMessage(BOT_TOKEN, CHAT_ID, buffer, (const char *)_binary_telegram_cert_pem_start);
        UartSendString(UART_PC, "Mensaje enviado al bot de Telegram\n");
    }
}

/**
* @brief Tarea encargada de la medición de la resistencia con AC
*/
static void measuringResistanceFunction(void *pvParameter){
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Leer valor del ADC
        AnalogInputReadSingle(CH0, &adc_raw);

        // Convertir a voltaje
        v_adc = ((float)adc_raw/1000);

        // Calcular resistencia del agua si está en un rango válido
        if (v_adc < 3.3) {
            r_water = ((v_adc * R1) / (3.3 - v_adc))*2.83; //2,83 es el valor de calibración.
            conductivity = (2.27 / r_water) * 1000000; // Convertir a µS/cm. Constante de celda: 2,27 1/cm
            water_hardness = CONVERSION_FACTOR*conductivity;
        }
        UartSendString(UART_PC, buffer);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void) {
    
    printf("P.Electronics System\n");

    //Parte del main que corresponde a la medición de la resistencia    
    serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&my_uart);	

    analog_input_config_t value = {
        .input = CH0,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0
    };
    AnalogInputInit(&value);

    timer_config_t timer_for_measure = {
        .timer = TIMER_A,
        .period = CONFIG_MEASURE_PERIOD,
        .func_p = measuringResistanceCallBack,
        .param_p = NULL
    };
    TimerInit(&timer_for_measure);

    timer_config_t timer_for_telegram = {
        .timer = TIMER_B,
        .period = CONFIG_TELEGRAM_PERIOD,
        .func_p = telegramMessageCallBack,
        .param_p = NULL,
    };
    TimerInit(&timer_for_telegram);

    ServoInit(SERVO_0,GPIO_4);

    WifiInit(WIFI_SSID, WIFI_PASS);
    vTaskDelay(pdMS_TO_TICKS(3000)); // Esperar conexión

    PWMInit(PWM_0, GPIO_9, 1000);     
    PWMSetDutyCycle(PWM_0, 50);
    PWMOn(PWM_0);

    xTaskCreate(&measuringResistanceFunction,"measureResistanceFunction",2048,NULL,5,&measuringResistanceTask);
    xTaskCreate(&telegramMessageFunction,"telegramMessageFunction",4096,NULL,5,&telegramMessageTask);

    TimerStart(timer_for_measure.timer);
    TimerStart(timer_for_telegram.timer);
}
/*==================[end of file]============================================*/