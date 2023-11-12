#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "timer_test.h"

const char *app_tag = "main";

// timer related definitions
esp_timer_create_args_t timer_args = {
        .callback = &timer_event_post
};
esp_timer_handle_t timer;

void app_main(void)
{
    ESP_LOGD(app_tag, "Main application startup\n");

    // get info about the soc
    uint32_t flash_size;
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        ESP_LOGD(app_tag, "Get flash size failed");
        return;
    }

    ESP_LOGD(app_tag, "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGD(app_tag, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // create default loop for all events
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // create test timer
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, TIMER_PERIOD));

    // register callback which event loop execute when timer event occurs
    ESP_ERROR_CHECK(esp_event_handler_instance_register(TIMER_EVENT, TIMER_EVENT_POST, timer_event_from_loop, NULL, NULL));

    // TODO wifi driver
    // TODO cam driver
}
