#ifndef ESP32_CAM_SCANNER_AI_MANAGER_H
#define ESP32_CAM_SCANNER_AI_MANAGER_H

#include "esp_event.h"
#include "esp_camera.h"

// deprecated
#define IMG_CHANNELS 3
#define IMG_WIDTH 512
#define IMG_HEIGHT 512

typedef enum {
    AI_EVENT_TASK_DONE,
    AI_EVENT_TASK_FAIL,
    AI_EVENT_ANY
} ai_events_t;

ESP_EVENT_DECLARE_BASE(AI_EVENTS);

#ifdef __cplusplus
extern "C" {
#endif
    void ai_init(void);
    void ai_task(void *pvParameters);
#ifdef __cplusplus
}
#endif

#endif //ESP32_CAM_SCANNER_AI_MANAGER_H
