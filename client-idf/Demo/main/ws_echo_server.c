
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_websocket_client.h"
#include "protocol_examples_common.h"

static const char *TAG = "WS_CLIENT";

/* WebSocket配置 *///ws://192.168.10.2:5000/
//#define WEBSOCKET_URI "ws://192.168.10.2:5000/"
#define WEBSOCKET_URI "ws://192.168.10.2:8088/?v=1.0.0&device_id=94:BB:43:ED:67:98"
// 如需使用SSL（wss://），请使用：
// #define WEBSOCKET_URI "wss://echo.websocket.org"

static esp_websocket_client_handle_t client = NULL;

/* WebSocket事件处理 */
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket 连接成功");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WebSocket 连接断开");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "[w]接收数据=%d", data->op_code);
            if (data->op_code == 0x08 && data->data_len == 2) {
                ESP_LOGI(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
            } else {
                ESP_LOGI(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WebSocket 错误");
            break;
    }
}

void app_main_websocket(void)
{
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化TCP/IP和事件循环
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 连接WiFi（需要先配置示例的WiFi设置）
    ESP_ERROR_CHECK(example_connect());

    // WebSocket客户端配置
    esp_websocket_client_config_t ws_config = {
        .uri = WEBSOCKET_URI, //ID+KEY+自定义
        .disable_auto_reconnect = false,
        .reconnect_timeout_ms = 10000,
        // 如果是wss://，需要配置SSL参数：
        // .client_cert = NULL,
        // .client_key = NULL,
        // .cert_pem = NULL,
        // .skip_cert_common_name_check = true
    };

    // 创建WebSocket客户端
    client = esp_websocket_client_init(&ws_config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return;
    }

    // 注册事件处理器
    ESP_ERROR_CHECK(esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, client));

    // 启动WebSocket连接
    esp_websocket_client_start(client);

    // 发送测试消息
    int i = 0;
    while (1) {
        if (esp_websocket_client_is_connected(client)) {
            char message[64];
            snprintf(message, sizeof(message), "Hello from ESP32S3 %d", i++);
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            ESP_LOGI(TAG, "Sent: %s", message);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    // 清理（通常不会执行到这里）
    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
}


void wakeUp(void)
{
    #if 0
    if (esp_websocket_client_is_connected(client)) {
        char message[64];
        snprintf(message, sizeof(message), "Hello from ESP32S3 %d", i++);
        esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
        ESP_LOGI(TAG, "Sent: %s", message);
    }
    #endif
}