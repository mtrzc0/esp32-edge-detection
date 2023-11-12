#include "esp_timer.h"
#include "esp_event.h"
#include "esp_log.h"
#include "timer_test.h"

const char *timer_tag = "timer";

ESP_EVENT_DEFINE_BASE(TIMER_EVENT);

void timer_event_post(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    (void) handler_arg;
    (void) event_data;
    ESP_LOGD(timer_tag, "Timer posting to event loop...");
    esp_event_post(TIMER_EVENT, TIMER_EVENT_POST, NULL, 0, portMAX_DELAY);
}

void timer_event_from_loop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    (void) handler_arg;
    (void) event_data;
    ESP_LOGD(timer_tag, "Event from event loop...");
}

