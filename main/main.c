#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "camera_manager.h"
#include "websocket_manager.h"
#include "ai_manager.h"

const char *app_tag = "app";

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

    // initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGD(app_tag, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // create default loop for all events
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(app_tag, "Initializing modules");
    wifi_init();
    websocket_init();
    camera_init();
    ai_init();
}
