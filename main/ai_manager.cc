#include <esp_log.h>
#include <esp_heap_caps.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "model_data.h"

#include "camera_manager.h"
#include "ai_manager.h"
#include "websocket_manager.h"
#include "task_prio.h"

static const char *ai_tag = "ai";

ESP_EVENT_DEFINE_BASE(AI_EVENTS);

namespace
{
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input, *output = nullptr;

    // FIXME: check if this is correct size for tensor arena, should be hned_tflite_len?
    constexpr int kTensorArenaSize = 50 * 1024;
    uint8_t *tensor_arena = nullptr;
}

extern "C" void ai_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void) arg;
    (void) event_data;
    if (event_base == AI_EVENTS && event_id == AI_EVENT_DONE)
    {
        BaseType_t ret = xTaskCreate(websocket_send,
                                     ai_tag,
                                     configMINIMAL_STACK_SIZE + 2048,
                                     pic,
                                     TP_WEBSOCKET_SEND,
                                     NULL);
        ESP_ERROR_CHECK(ret != pdPASS ? ESP_ERR_NO_MEM : ESP_OK);
        ESP_LOGI(ai_tag, "AI done");
    }
    else if (event_base == AI_EVENTS && event_id == AI_EVENT_FAIL)
    {
        ESP_LOGE(ai_tag, "AI failed");
    }
}

extern "C" void ai_init()
{
    esp_event_handler_instance_register(AI_EVENTS,
                                        AI_EVENT_DONE,
                                        (esp_event_handler_t) ai_event_handler,
                                        NULL,
                                        NULL);

    model = tflite::GetModel(hned_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        ESP_LOGD(ai_tag, "Model provided is schema version %lu not equal to supported version %d.", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // allocate buffer for tensor arena in SPIRAM
    // FIXME: tensor arena size is not correct
    tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);
    if (tensor_arena == nullptr)
    {
        ESP_LOGD(ai_tag, "Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
        return;
    }
    else
    {
        ESP_LOGD(ai_tag, "Allocated memory of %d bytes\n", kTensorArenaSize);
    }

    // FIXME: check if this is correct setup
    static tflite::MicroMutableOpResolver<5> micro_op_resolver;
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();

    static tflite::MicroInterpreter static_interpreter(model,
                                                       micro_op_resolver,
                                                       tensor_arena,
                                                       kTensorArenaSize);
    // FIXME: AllocateTensors() fails
    interpreter = &static_interpreter;
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        ESP_LOGD(ai_tag, "AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
}

extern "C" void ai_run(void *pvParameters)
{
    (void) pvParameters;
    input->data.uint8 = pic->buf;

    // run the model with img data
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        esp_event_post(AI_EVENTS,
                       AI_EVENT_FAIL,
                       NULL,
                       0,
                       portMAX_DELAY);
    }
    else
    {
        esp_event_post(AI_EVENTS,
                       AI_EVENT_DONE,
                       NULL,
                       0,
                       portMAX_DELAY);
        // save the result
        output = interpreter->output(0);
        // avoid watchdog trigger
        vTaskDelay(1);
        // update picture buffer
        pic->buf = output->data.uint8;
    }
}
