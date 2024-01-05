#include <esp_log.h>
#include <esp_heap_caps.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "model_data.h"
#include "esp_camera.h"

#include "camera_manager.h"
#include "ai_manager.h"

static const char *ai_tag = "ai";

namespace
{
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;

    constexpr int kTensorArenaSize = 60 * 1024;
    static uint8_t *tensor_arena;

    constexpr int kImageSize = 512;
    constexpr int kImageChannels = 3;
}

extern "C" void ai_init()
{
    model = tflite::GetModel(hned_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        ESP_LOGD(ai_tag, "Model provided is schema version %lu not equal to supported version %d.", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    if (tensor_arena == nullptr)
    {
        tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        ESP_LOGD(ai_tag, "Could not allocate arena");
        return;
    }

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
    interpreter = &static_interpreter;
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        ESP_LOGD(ai_tag, "AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
}

extern "C" void ai_run()
{
    input->data.uint8 = pic->buf;

    // run the model with img data
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        ESP_LOGD(ai_tag, "Invoke failed");
        return;
    }

    // save the result
    TfLiteTensor *output = interpreter->output(0);
    // avoid watchdog trigger
    vTaskDelay(1);

    pic->buf = output->data.uint8;
}
