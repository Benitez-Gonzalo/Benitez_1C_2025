#ifndef TELEGRAM_BOT_MCU_H
#define TELEGRAM_BOT_MCU_H

#include <stdbool.h>

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
