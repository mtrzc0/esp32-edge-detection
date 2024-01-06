#ifndef ESP32_CAM_SCANNER_AI_MANAGER_H
#define ESP32_CAM_SCANNER_AI_MANAGER_H

#include "esp_event.h"

#define KTENSORSARENA_BYTES 50

typedef enum {
    AI_EVENT_DONE,
    AI_EVENT_FAIL,
    AI_EVENT_ANY
} ai_events_t;

ESP_EVENT_DECLARE_BASE(AI_EVENTS);

#ifdef __cplusplus
extern "C" {
#endif
    void ai_init(void);
    void ai_run(void *pvParameters);
#ifdef __cplusplus
}
#endif

#endif //ESP32_CAM_SCANNER_AI_MANAGER_H
