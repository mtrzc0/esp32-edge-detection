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
    (void) base;
    (void) id;
    ESP_LOGD(timer_tag, "Timer posting to event loop...");
    esp_event_post(TIMER_EVENT, TIMER_EVENT_POST, NULL, 0, portMAX_DELAY);
}

void timer_event_from_loop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    (void) handler_arg;
    (void) event_data;
    ESP_LOGD(timer_tag, "Event from event loop...");
}

void timer_init(void)
{
    // timer related definitions
    esp_timer_create_args_t timer_args = {
            .callback = (esp_timer_cb_t) &timer_event_post
    };
    esp_timer_handle_t timer;



    // create test timer
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, TIMER_PERIOD));

    // register callback which event loop execute when timer event occurs
    ESP_ERROR_CHECK(esp_event_handler_instance_register(TIMER_EVENT, TIMER_EVENT_POST, timer_event_from_loop, NULL, NULL));
}

