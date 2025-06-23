#ifndef TELEGRAM_BOT_MCU_H
#define TELEGRAM_BOT_MCU_H

#include <stdbool.h>

/** \brief Inicializa la conexión Wi-Fi
 *
 * @param ssid Nombre de la red Wi-Fi
 * @param password Contraseña de la red Wi-Fi
 * @return true si se conecta correctamente, false en caso contrario
 */
bool WifiInit(const char *ssid, const char *password);

/** \brief Envía un mensaje al bot de Telegram
 *
 * @param token Token del bot
 * @param chat_id ID del chat al que enviar el mensaje
 * @param message Texto a enviar
 * @param cert_pem Certificado .pem
 * @return true si el mensaje fue enviado correctamente, false en caso de error
 */
bool TelegramSendMessage(const char *token, const char *chat_id, const char *message, const char *cert_pem);

#endif // TELEGRAM_BOT_MCU_H
