#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"

void app_main(void)
{
    const char *app_tag = "main_app";
    ESP_LOGD(app_tag, "Main application startup\n");

    uint32_t flash_size;
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        ESP_LOGD(app_tag, "Get flash size failed");
        return;
    }

    ESP_LOGD(app_tag, "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGD(app_tag, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    //TODO main event loop
    //TODO project dirs refactor

    for (int i = 10; i >= 0; i--) {
        ESP_LOGD(app_tag, "Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGD(app_tag, "Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
