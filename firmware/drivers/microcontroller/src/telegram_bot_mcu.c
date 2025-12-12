#include "telegram_bot_mcu.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_netif.h"
#include <string.h>

bool TelegramSendMessage(const char *token, const char *chat_id, const char *message, const char *cert_pem) {
    char url[512];
    snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage", token);

    char post_data[512];
    snprintf(post_data, sizeof(post_data),"chat_id=%s&text=%s", chat_id, message);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = cert_pem,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Mensaje enviado correctamente: %s", post_data);
        esp_http_client_cleanup(client);
        return true;
    } else {
        ESP_LOGE(TAG, "Error al enviar mensaje: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }
}
