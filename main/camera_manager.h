#ifndef ESP32_CAM_SCANNER_CAMERA_MANAGER_H
#define ESP32_CAM_SCANNER_CAMERA_MANAGER_H

#if CONFIG_OV2640_SUPPORT
#include "ov2640.h"
#endif
#if CONFIG_OV7725_SUPPORT
#include "ov7725.h"
#endif
#if CONFIG_OV3660_SUPPORT
#include "ov3660.h"
#endif
#if CONFIG_OV5640_SUPPORT
#include "ov5640.h"
#endif
#if CONFIG_NT99141_SUPPORT
#include "nt99141.h"
#endif
#if CONFIG_OV7670_SUPPORT
#include "ov7670.h"
#endif
#if CONFIG_GC2145_SUPPORT
#include "gc2145.h"
#endif
#if CONFIG_GC032A_SUPPORT
#include "gc032a.h"
#endif
#if CONFIG_GC0308_SUPPORT
#include "gc0308.h"
#endif
#if CONFIG_BF3005_SUPPORT
#include "bf3005.h"
#endif
#if CONFIG_BF20A6_SUPPORT
#include "bf20a6.h"
#endif
#if CONFIG_SC101IOT_SUPPORT
#include "sc101iot.h"
#endif
#if CONFIG_SC030IOT_SUPPORT
#include "sc030iot.h"
#endif
#if CONFIG_SC031GS_SUPPORT
#include "sc031gs.h"
#endif

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define TAG ""
#else
#include "esp_log.h"
static const char *TAG = "camera";
#endif

typedef struct {
    sensor_t sensor;
    camera_fb_t fb;
} camera_state_t;

static const char *CAMERA_SENSOR_NVS_KEY = "sensor";
static const char *CAMERA_PIXFORMAT_NVS_KEY = "pixformat";
static camera_state_t *s_state = NULL;

#if CONFIG_IDF_TARGET_ESP32S3 // LCD_CAM module of ESP32-S3 will generate xclk
#define CAMERA_ENABLE_OUT_CLOCK(v)
#define CAMERA_DISABLE_OUT_CLOCK()
#else
#define CAMERA_ENABLE_OUT_CLOCK(v) camera_enable_out_clock((v))
#define CAMERA_DISABLE_OUT_CLOCK() camera_disable_out_clock()
#endif

typedef struct {
    int (*detect)(int slv_addr, sensor_id_t *id);
    int (*init)(sensor_t *sensor);
} sensor_func_t;

static const sensor_func_t g_sensors[] = {
#if CONFIG_OV7725_SUPPORT
        {ov7725_detect, ov7725_init},
#endif
#if CONFIG_OV7670_SUPPORT
        {ov7670_detect, ov7670_init},
#endif
#if CONFIG_OV2640_SUPPORT
        {ov2640_detect, ov2640_init},
#endif
#if CONFIG_OV3660_SUPPORT
        {ov3660_detect, ov3660_init},
#endif
#if CONFIG_OV5640_SUPPORT
        {ov5640_detect, ov5640_init},
#endif
#if CONFIG_NT99141_SUPPORT
        {nt99141_detect, nt99141_init},
#endif
#if CONFIG_GC2145_SUPPORT
        {gc2145_detect, gc2145_init},
#endif
#if CONFIG_GC032A_SUPPORT
        {gc032a_detect, gc032a_init},
#endif
#if CONFIG_GC0308_SUPPORT
        {gc0308_detect, gc0308_init},
#endif
#if CONFIG_BF3005_SUPPORT
        {bf3005_detect, bf3005_init},
#endif
#if CONFIG_BF20A6_SUPPORT
        {bf20a6_detect, bf20a6_init},
#endif
#if CONFIG_SC101IOT_SUPPORT
        {sc101iot_detect, sc101iot_init},
#endif
#if CONFIG_SC030IOT_SUPPORT
        {sc030iot_detect, sc030iot_init},
#endif
#if CONFIG_SC031GS_SUPPORT
        {sc031gs_detect, sc031gs_init},
#endif
};

#endif //ESP32_CAM_SCANNER_CAMERA_MANAGER_H
