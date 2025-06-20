#pragma once

#include "esp_websocket_client.h"


#include <stdbool.h>

#define MAX_FIELDS     16
#define MAX_KEY_LEN    32
#define MAX_VALUE_LEN  128

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
} ws_field_t;

typedef struct {
    char type[MAX_VALUE_LEN];        // 提取 type 字段
    ws_field_t fields[MAX_FIELDS];   // 提取所有字符串/数字字段
    int field_count;
} ws_msg_info_t;

bool parse_ws_message(const char *json_str, ws_msg_info_t *msg_info);
void websocket_app_start(void);
bool parse_ws_message(const char *json_str, ws_msg_info_t *msg_info);
