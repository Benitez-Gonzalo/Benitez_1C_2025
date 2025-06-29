
/*! @mainpage Final Project
 *
 * @section genDesc General Description
 *
 * This program measures water hardness (CaCO3 concentration) based on a voltage divider with an ESP32-C6-DevkitC-1.
 * A known resistance (R1 = 10 kOhm) connected to a potentiometer simulates the salt concentration variation and 
 * electrodes into the water sense its impedance; that value is used to make the calculations. 
 * The electrodes voltage is read using the analog_io_mcu driver, and the hardness is calculated and displayed via 
 * the serial port. Then, that value is send through a Telegram bot periodically. If it is is too high, a filter is 
 * deployed into the water sample. Momentarilly, the filter is represented as an LED due to the ESP lack of power.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Component / Node                      |   ESP32-C6          |
 * |:----------------------------------------:|:-------------------:|
 * | Capacitor (104) to R1=10 kOhm to a pot   |  GPIO9 (PWM output) |
 * | to the anode electrode.                  |                     |
 * | Junction between R1 and potentiometer    |  GPIO0 (ADC CH0)    |
 * | Negative Electrode (Cathode)             |  GND                |
 * | LED                                      |  GPIO11             |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | April 2025 | Document creation		                         |
 * | 12/05/2025 | Using analog_io_mcu driver for ADC readings    |
 * | 12/06/2025 | Created Telegram Bot with its driver           |
 * | 18/06/2025 | The flash partition size was increased         |
 * | 21/06/2025 | PWM was used to impedance measurements         |
 * | 22/06/2025 | LED driving was implemented                    |
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
#include "led.h"
#include "pwm_mcu.h"
/*==================[macros and definitions]=================================*/
#define R1  10000.0 // Ohm (resistencia conocida)
#define CONFIG_MEASURE_PERIOD 1000000
#define CONFIG_TELEGRAM_PERIOD 3000000
#define WIFI_SSID "moto-e(7)-Gonza"
#define WIFI_PASS "gonza235"
#define BOT_TOKEN "7666661023:AAGC_aNd2ElAc5ieqiJxhCb8IVP74LCZG1o"
#define CHAT_ID "7725635002" 
#define UMBRAL 10 //Umbral de dureza en mg/L
#define CONVERSION_FACTOR 0.145
/*==================[internal data definition]===============================*/
TaskHandle_t measuringHardnessTask = NULL;
TaskHandle_t telegramMessageTask = NULL;
uint16_t voltage = 0;
uint32_t resistance = 0;
uint16_t adc_raw = 0;
float v_adc = 0;
float r_water = 0;
float conductivity = 0;
float water_hardness = 0;
char buffer[100];
extern const uint8_t _binary_telegram_cert_pem_start[];
static uint8_t led_state = 255; // Inicialmente inválido
/*==================[internal functions declaration]=========================*/

void measuringHardnessCallBack (void){
    vTaskNotifyGiveFromISR(measuringHardnessTask,pdFALSE);
}

void telegramMessageCallBack (void){
    vTaskNotifyGiveFromISR(telegramMessageTask,pdFALSE);
}

/**
 * @brief Tarea encargada de enviar el mensaje vía Telegram
*/
static void telegramMessageFunction(void *pvParameter) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        snprintf(buffer, sizeof(buffer),"Dureza (CaCO3)  = %.0f mg/L\n", water_hardness);

        TelegramSendMessage(BOT_TOKEN, CHAT_ID, buffer, (const char *)_binary_telegram_cert_pem_start);
        UartSendString(UART_PC, "Mensaje enviado al bot de Telegram\n");
    }
}

/**
* @brief Tarea encargada de la medición de la dureza
*/
static void measuringHardnessFunction(void *pvParameter){
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Leer valor del ADC
        AnalogInputReadSingle(CH0, &adc_raw);

        // Convertir a voltaje
        v_adc = ((float)adc_raw/1000);

        // Calcular si se está en un voltaje válido
        if (v_adc < 3.3) {
            r_water = ((v_adc * R1) / (3.3 - v_adc))*2.83; //2,83 es el valor de calibración. Se obtuvo experimentalmente.
            conductivity = (2.27 / r_water) * 1000000; // Convertir a µS/cm. Constante de celda: 2,27 [1/cm]
            water_hardness = CONVERSION_FACTOR*conductivity;

            uint8_t new_led_state = (water_hardness > UMBRAL) ? 1 : 0;

            if (new_led_state != led_state) {
                if (new_led_state) {
                    LedOn(LED_1);
                } else {
                    LedOff(LED_1);
                }
                led_state = new_led_state;
            }
        }
        UartSendString(UART_PC, buffer);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void) {
      
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
        .func_p = measuringHardnessCallBack,
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

    WifiInit(WIFI_SSID, WIFI_PASS);
    vTaskDelay(pdMS_TO_TICKS(3000)); // Esperar conexión

    LedsInit();

    PWMInit(PWM_0, GPIO_9, 1000);     
    PWMSetDutyCycle(PWM_0, 50);
    PWMOn(PWM_0);

    xTaskCreate(&measuringHardnessFunction,"measuringHardnessFunction",2048,NULL,5,&measuringHardnessTask);
    xTaskCreate(&telegramMessageFunction,"telegramMessageFunction",4096,NULL,5,&telegramMessageTask);

    TimerStart(timer_for_measure.timer);
    TimerStart(timer_for_telegram.timer);
}
/*==================[end of file]============================================*/