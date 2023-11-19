#ifndef ESP32_CAM_SCANNER_WEBSOCKET_MANAGER_H
#define ESP32_CAM_SCANNER_WEBSOCKET_MANAGER_H

#include <esp_http_client.h>
#include "esp_camera.h"

void udp_client_sock_init(void);
void udp_client_task(void *pvParameters);

#endif //ESP32_CAM_SCANNER_WEBSOCKET_MANAGER_H
