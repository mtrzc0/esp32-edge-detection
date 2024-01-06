#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
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

void websocket_init(void)
{
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

    sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0)
        ESP_LOGE(websocket_tag, "Unable to create socket: errno %d", errno);
    else
        ESP_LOGI(websocket_tag, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
}

void websocket_send(void *pvParameters)
{
    camera_fb_t *pic = pvParameters;
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    ESP_LOGD(websocket_tag, "pic->len: %d", pic->len);
    int err = sendto(sock, (void*) pic->buf, pic->len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
        ESP_LOGE(websocket_tag, "Error occurred during sending: errno %d", errno);
    else
        ESP_LOGI(websocket_tag, "JPEG binary sent in UDP");

    vTaskDelete(NULL);
}