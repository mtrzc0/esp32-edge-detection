#ifndef ESP32_CAM_SCANNER_WEBSOCKET_MANAGER_H
#define ESP32_CAM_SCANNER_WEBSOCKET_MANAGER_H

#include <esp_http_client.h>
#include "esp_camera.h"

void websocket_init(void);
void websocket_send(void *pvParameters);

#endif //ESP32_CAM_SCANNER_WEBSOCKET_MANAGER_H
