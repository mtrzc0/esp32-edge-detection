#include <esp_log.h>
#include <esp_err.h>
#include "camera_manager.h"
#include "task_prio.h"

static const char *cam_tag = "camera";
TaskFunction_t take_picture_task = NULL;

esp_err_t camera_init(void)
{
    //fixme init error
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(cam_tag, "Camera Init Failed");
        return err;
    }

    BaseType_t ret;

    // create task which takes picture
    ret = xTaskCreate(take_picture_task, cam_tag, configMINIMAL_STACK_SIZE, NULL, TP_TAKE_PIC, NULL);
    ESP_ERROR_CHECK(ret != pdPASS ? ESP_ERR_NO_MEM : ESP_OK);

    return ESP_OK;
}

void take_picture(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(cam_tag, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // use pic->buf to access the image
        ESP_LOGI(cam_tag, "Picture taken! Its size was: %zu bytes", pic->len);
        esp_camera_fb_return(pic);

        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}