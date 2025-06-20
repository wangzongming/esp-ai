#include "websocket_rx_handler.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "WS_HANDLER";

#define MAX_JSON_LENGTH 1024*11
#define JSON_QUEUE_SIZE 5

QueueHandle_t json_msg_queue;

extern esp_websocket_client_handle_t client ;
bool parse_ws_message(const char *json_str, ws_msg_info_t *msg_info)
{
    if (!json_str || !msg_info) return false;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
      //  ESP_LOGE(TAG, "JSON 解析失败: %s", json_str);
        return false;
    }

    memset(msg_info, 0, sizeof(ws_msg_info_t));

    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (!cJSON_IsString(type)) {
        cJSON_Delete(root);
        return false;  // 无 type 字段则忽略
    }

    strncpy(msg_info->type, type->valuestring, MAX_VALUE_LEN - 1);

    cJSON *item = NULL;
    int index = 0;
    cJSON_ArrayForEach(item, root) {
        if (index >= MAX_FIELDS) break;

        const char *key = item->string;
        char value_buf[MAX_VALUE_LEN] = {0};

        if (cJSON_IsString(item)) {
            strncpy(value_buf, item->valuestring, MAX_VALUE_LEN - 1);
        } else if (cJSON_IsNumber(item)) {
            snprintf(value_buf, sizeof(value_buf), "%.0f", item->valuedouble);
        } else {
            continue;  // 跳过非字符串/数字类型
        }

        if (key) {
            strncpy(msg_info->fields[index].key, key, MAX_KEY_LEN - 1);
            strncpy(msg_info->fields[index].value, value_buf, MAX_VALUE_LEN - 1);
            index++;
        }
    }

    msg_info->field_count = index;
    cJSON_Delete(root);
    return true;
}

char Lock_SendPCM=0;
// JSON 处理任务
static void json_parse_task(void *arg)
{
    char *recv_buf = NULL;
    ws_msg_info_t msg_info;

    while (1) {
       if (xQueueReceive(json_msg_queue, &recv_buf, portMAX_DELAY) == pdPASS) {
           // ESP_LOGI(TAG, "收到 WebSocket 数据: %s", recv_buf);

            if (parse_ws_message(recv_buf, &msg_info)) {
               // ESP_LOGI(TAG, "解析成功，type = %s", msg_info.type);
                for (int i = 0; i < msg_info.field_count; i++) {
                    ESP_LOGI(TAG, "字段 %s = %s", msg_info.fields[i].key, msg_info.fields[i].value);
                }
                // 👉 按 type 分发
                if (strcmp(msg_info.type, "session_start") == 0) {
                  //  ESP_LOGI(TAG, "触发会话开始逻辑");
                    // 可以使用 msg_info.fields 中的值
                } else if (strcmp(msg_info.type, "play_audio") == 0) {
                   // ESP_LOGI(TAG, "开始播放音频");
                } else if (strcmp(msg_info.type, "session_stop") == 0) {
                   // ESP_LOGI(TAG, "🛑收到会话停止指令，执行清理逻辑");
                    char message[64];
                    snprintf(message, sizeof(message), "{ \"type\":\"session_stop_ack\", \"session_id\": \"0\"}");
                  //  ESP_LOGI(TAG, "发送开始消息: %s", message);
                     esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
                Lock_SendPCM=1;
                    } else {
                  //  ESP_LOGI(TAG, "其他类型处理: %s", msg_info.type);
                }
            } else {
               // ESP_LOGW(TAG, "无法解析 JSON 或缺少 type 字段");
            }

            free(recv_buf);
        }
    }
}

// 启动 WebSocket 客户端与任务
void websocket_app_start(void)
{
    json_msg_queue = xQueueCreate(JSON_QUEUE_SIZE, sizeof(char *));
    xTaskCreate(json_parse_task, "json_parse_task", 1024*10, NULL, 5, NULL);
}
