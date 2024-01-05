#include <esp_log.h>
#include <esp_err.h>
#include "camera_manager.h"
#include "task_prio.h"
#include "websocket_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *camera_tag = "camera";

ESP_EVENT_DEFINE_BASE(CAMERA_EVENTS);
camera_fb_t *pic;

static void camera_event_handler(void *arg, esp_event_base_t event_base, int8_t event_id, void *event_data)
{
    (void) arg;
    (void) event_data;
    if (event_base == CAMERA_EVENTS && event_id == CAMERA_EVENT_INIT_FAIL)
    {
        ESP_LOGE(camera_tag, "Camera Init Failed");
    }
    else if (event_base == CAMERA_EVENTS && event_id == CAMERA_EVENT_READY)
    {
        ESP_LOGI(camera_tag, "Camera ready");
        // create task which takes picture
        BaseType_t ret = xTaskCreate(take_picture,
                                     camera_tag,
                                     configMINIMAL_STACK_SIZE + 2048,
                                     NULL,
                                     TP_TAKE_PIC,
                                     NULL);
        ESP_ERROR_CHECK(ret != pdPASS ? ESP_ERR_NO_MEM : ESP_OK);
    }
    else if (event_base == CAMERA_EVENTS && event_id == CAMERA_EVENT_PICTURE_TAKEN)
    {
        ESP_LOGI(camera_tag, "Picture taken! Its size was: %zu bytes", pic->len);
        BaseType_t ret = xTaskCreate(udp_client_task,
                                     camera_tag,
                                     configMINIMAL_STACK_SIZE + 2048,
                                     pic,
                                     TP_SOCKET_SEND_PIC,
                                     NULL);
        ESP_ERROR_CHECK(ret != pdPASS ? ESP_ERR_NO_MEM : ESP_OK);
    }
}

esp_err_t camera_init(void)
{
    // register all camera events
    esp_event_handler_register(CAMERA_EVENTS,
                               CAMERA_EVENT_PICTURE_TAKEN,
                               (esp_event_handler_t) camera_event_handler,
                               NULL);

    esp_event_handler_instance_register(CAMERA_EVENTS,
                                        CAMERA_EVENT_INIT_FAIL,
                                        (esp_event_handler_t) camera_event_handler,
                                        NULL,
                                        NULL);

    esp_event_handler_instance_register(CAMERA_EVENTS,
                                        CAMERA_EVENT_READY,
                                        (esp_event_handler_t) camera_event_handler,
                                        NULL,
                                        NULL);
    // initialize the camera
    esp_err_t ret = esp_camera_init(&camera_config);
    if (ret != ESP_OK)
    {
        esp_event_post(CAMERA_EVENTS,
                          CAMERA_EVENT_INIT_FAIL,
                          NULL,
                          0,
                          portMAX_DELAY);
        return ret;
    } else {
        esp_event_post(CAMERA_EVENTS,
                          CAMERA_EVENT_READY,
                          NULL,
                          0,
                          portMAX_DELAY);
        return ret;
    }
}

void take_picture(void *pvParameters)
{
    (void) pvParameters;
    for(;;)
    {
        ESP_LOGI(camera_tag, "Taking picture...");
        pic = esp_camera_fb_get();
        ESP_LOGD(camera_tag, "pic->len: %d", pic->len);
        esp_event_post(CAMERA_EVENTS,
                       CAMERA_EVENT_PICTURE_TAKEN,
                       NULL,
                       0,
                       portMAX_DELAY);
        esp_camera_fb_return(pic);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}