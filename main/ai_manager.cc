#include <esp_log.h>
#include <esp_heap_caps.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"

#include "model_data_test.h"

#include "camera_manager.h"
#include "ai_manager.h"
#include "websocket_manager.h"
#include "task_prio.h"

static const char *ai_tag = "ai";

ESP_EVENT_DEFINE_BASE(AI_EVENTS);

namespace
{
    const tflite::Model *model = nullptr;
    tflite::RecordingMicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input, *output = nullptr;

    // tensor arena should be 3 (???) (img) * (channel) * (height) * (width)
    const int kTensorArenaSize = 3 * CONFIG_AI_IMAGE_CHANNELS * CONFIG_AI_IMAGE_HEIGHT * CONFIG_AI_IMAGE_WIDTH;
    uint8_t *tensor_arena = nullptr;
    SemaphoreHandle_t ai_mutex;
    camera_fb_t *pic;
}

extern "C" void ai_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void) arg;
    (void) event_data;
    if (event_base == AI_EVENTS && event_id == AI_EVENT_TASK_FAIL) {
        ESP_LOGE(ai_tag, "Task failed");
    }
    else if (event_base == AI_EVENTS && event_id == AI_EVENT_TASK_DONE)
    {
        ESP_LOGD(ai_tag, "Picture size before sending is %d bytes at address %p", pic->len, pic);
        // pin sending image to core 1
        BaseType_t ret = xTaskCreate(websocket_send,
                                     ai_tag,
                                     configMINIMAL_STACK_SIZE + 2048,
                                     pic,
                                     TP_WEBSOCKET_SEND,
                                     nullptr);
        ESP_ERROR_CHECK(ret != pdPASS ? ESP_ERR_NO_MEM : ESP_OK);
    }
}

extern "C" void ai_init()
{
    esp_event_handler_instance_register(AI_EVENTS,
                                        AI_EVENT_TASK_DONE,
                                        (esp_event_handler_t) ai_event_handler,
                                        nullptr,
                                        nullptr);

    esp_event_handler_instance_register(AI_EVENTS,
                                        AI_EVENT_TASK_FAIL,
                                        (esp_event_handler_t) ai_event_handler,
                                        nullptr,
                                        nullptr);

    ai_mutex = xSemaphoreCreateMutex();
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
    ESP_LOGD(ai_tag, "Allocated memory of %d bytes\n", kTensorArenaSize);

    // TODO: check if this is correct setup
    static tflite::MicroMutableOpResolver<20> micro_op_resolver;
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();
    micro_op_resolver.AddMul();
    micro_op_resolver.AddSub();
    micro_op_resolver.AddPad();
    micro_op_resolver.AddTranspose();
    micro_op_resolver.AddWhile();
    micro_op_resolver.AddConcatenation();
    micro_op_resolver.AddStridedSlice();
    micro_op_resolver.AddCast();
    micro_op_resolver.AddResizeBilinear();
    micro_op_resolver.AddLogistic();
    micro_op_resolver.AddLess();
    micro_op_resolver.AddAdd();
    micro_op_resolver.AddGather();
    micro_op_resolver.AddEqual();
    micro_op_resolver.AddIf();

    static tflite::RecordingMicroInterpreter static_interpreter(model,
                                                                micro_op_resolver,
                                                                tensor_arena,
                                                                kTensorArenaSize);

    interpreter = &static_interpreter;
    if (interpreter == nullptr)
    {
        ESP_LOGD(ai_tag, "Failed to create interpreter");
        return;
    }
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        ESP_LOGD(ai_tag, "AllocateTensors() failed");
        return;
    }

    // after AllocateTensors() find optimal arena size
    ESP_LOGD(ai_tag, "Optimal arena size: %d", interpreter->arena_used_bytes());

    // print allocations
    interpreter->GetMicroAllocator().PrintAllocations();

    // get input and output tensors
    input = interpreter->input(0);
    input->allocation_type = kTfLiteDynamic;
    input->type = kTfLiteUInt8;

}

extern "C" void ai_task(void *pvParameters)
{
    pic = (camera_fb_t *) pvParameters;
    ESP_LOGD(ai_tag, "Running on picture of size %d at address %p", pic->len, pic);

    // set input tensor to image data
    if (input->dims == nullptr)
    {
        ESP_LOGD(ai_tag, "Input tensor has no dimensions");
        return;
    }
    ESP_LOGD(ai_tag, "Input tensor has %d dimensions", input->dims->size);

    input->allocation_type = kTfLiteDynamic;
    input->type = kTfLiteUInt8;
    input->data.uint8 = pic->buf;
    input->bytes = pic->len;

    xSemaphoreTake(ai_mutex, portMAX_DELAY);
    // run the model with img data
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        esp_event_post(AI_EVENTS,
                       AI_EVENT_TASK_FAIL,
                       nullptr,
                       0,
                       portMAX_DELAY);
    }
    else
    {
        // save the result
        output = interpreter->output(0);
        // avoid watchdog trigger
        vTaskDelay(1);
        // update picture buffer
        pic->buf = output->data.uint8;
        pic->len = output->bytes;
        ESP_LOGD(ai_tag, "Shape of output tensor is %d x %d x %d", output->dims->data[0], output->dims->data[1], output->dims->data[2]);
        ESP_LOGD(ai_tag, "Picture size after processing is %d bytes at address %p", pic->len, pic);
        esp_event_post(AI_EVENTS,
                       AI_EVENT_TASK_DONE,
                       nullptr,
                       0,
                       portMAX_DELAY);
    }
    xSemaphoreGive(ai_mutex);
    vTaskDelete(nullptr);
}
