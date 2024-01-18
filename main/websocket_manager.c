#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#include "camera_manager.h"
#include "websocket_manager.h"

#include "esp_camera.h"

#if defined(CONFIG_IPV4)
#define HOST_IP_ADDR CONFIG_SOCKET_IPV4_ADDR
#elif defined(CONFIG_IPV6)
#define HOST_IP_ADDR CONFIG_SOCKET_IPV6_ADDR
#else
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_SOCKET_PORT

static const char *websocket_tag = "websocket";
static int32_t sock = 0;

static SemaphoreHandle_t websocket_mutex;

void websocket_init(void)
{
    websocket_mutex = xSemaphoreCreateMutex();
    ESP_LOGI(websocket_tag, "Initializing socket");
    ESP_LOGI(websocket_tag, "Socket address: %s:%d", HOST_IP_ADDR, PORT);

    int32_t addr_family = 0;
    int32_t ip_protocol = 0;

#if defined(CONFIG_IPV4)
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_IPV6)
    struct sockaddr_in6 dest_addr = { 0 };
    inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons(PORT);
    dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
    addr_family = AF_INET6;
    ip_protocol = IPPROTO_IPV6;
#endif

    sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0)
    {
        ESP_LOGE(websocket_tag, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(websocket_tag, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    int32_t conn = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (conn != 0)
    {
        ESP_LOGE(websocket_tag, "Socket unable to connect: errno %d", errno);
        return;
    }
    ESP_LOGI(websocket_tag, "Successfully connected");
}

void websocket_send(void *pvParameters)
{
    camera_fb_t *pic = pvParameters;
    xSemaphoreTake(websocket_mutex, portMAX_DELAY);
    if (pic == NULL)
    {
        ESP_LOGE(websocket_tag, "Picture buffer is NULL");
        goto retry_while_error;
    }
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    ESP_LOGI(websocket_tag, "Sending picture of size %d bytes at address %p", pic->len, pic);
    int32_t err = send(sock, (void*) pic->buf, pic->len, TCP_KEEPALIVE);
    if (err < 0)
    {
        ESP_LOGE(websocket_tag, "Error occurred during sending: errno %d", errno);
        ESP_LOGE(websocket_tag, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
        vTaskDelay(1);
        websocket_init();
        goto retry_while_error;
    }

    retry_while_error:
    // start taking new pictures
    esp_event_post(CAMERA_EVENTS, CAMERA_EVENT_TASK_START, NULL, 0, portMAX_DELAY);

    xSemaphoreGive(websocket_mutex);
    vTaskDelete(NULL);
}