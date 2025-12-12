#ifndef WIFI_MCU_H
#define WIFI_MCU_H

#include <stdbool.h>

/** \brief Inicializa la conexión Wi-Fi
 *
 * @param ssid Nombre de la red Wi-Fi
 * @param password Contraseña de la red Wi-Fi
 * @return true si se conecta correctamente, false en caso contrario
 */
bool WifiInit(const char *ssid, const char *password);

#endif // WIFI_MCU_H