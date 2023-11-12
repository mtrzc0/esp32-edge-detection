#ifndef TIMER_TEST_H_
#define TIMER_TEST_H_

#include "esp_event.h"
#include "esp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

// timer period in ms
#define TIMER_PERIOD (1000000)

typedef enum
{
    TIMER_EVENT_POST,
    ANY_EVENT
} timer_event_t;

ESP_EVENT_DECLARE_BASE(TIMER_EVENT);

void timer_event_post(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);
void timer_event_from_loop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);

#ifdef __cplusplus
}
#endif

#endif // TIMER_TEST_H_
