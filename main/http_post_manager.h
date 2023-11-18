#ifndef ESP32_CAM_SCANNER_HTTP_POST_MANAGER_H
#define ESP32_CAM_SCANNER_HTTP_POST_MANAGER_H

#include <esp_http_client.h>
#include "esp_camera.h"

esp_err_t http_event_handler(esp_http_client_event_t *evt);
static esp_http_client_config_t http_client_config = {
        .url = "http://192.168.43.170:8080",
        .host = "192.168.43.170",
        .port = 8080,
        .method = HTTP_METHOD_POST,
        .buffer_size = DEFAULT_HTTP_BUF_SIZE,
        .auth_type = HTTP_AUTH_TYPE_NONE,
        .is_async = 1,
        .event_handler = http_event_handler,
        .timeout_ms = 3000,
};


void http_async_post(void *pic);

#endif //ESP32_CAM_SCANNER_HTTP_POST_MANAGER_H
