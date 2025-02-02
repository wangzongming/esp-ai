/**
 * Copyright (c) 2024 小明IO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Commercial use of this software requires prior written authorization from the Licensor.
 * 请注意：将 ESP-AI 代码用于商业用途需要事先获得许可方的授权。
 * 删除与修改版权属于侵权行为，请尊重作者版权，避免产生不必要的纠纷。
 *
 * @author 小明IO
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */
#include "setWifiConfig.h"

/**
 * 重新设置 WiFi、api_key 信息， 设置后会重新连接wifi
 */
bool ESP_AI::setWifiConfig(JSONVar data)
{
    DEBUG_PRINTLN(debug, ("==================== 调用 setWifiConfig 方法设置信息 ===================="));
    JSONVar keys = data.keys();
    String wifi_name = data["wifi_name"];
    String wifi_pwd = data["wifi_pwd"];

    WiFi.begin(wifi_name, wifi_pwd);
    DEBUG_PRINT(debug, "connect wifi ing..");

    int connect_count = 0;
    // 10s 连不上Wifi的话
    int try_count = 15;
    while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
    {
        connect_count++;
        // 内置状态处理
        status_change("0_ing");
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing");
        }
        delay(250);
        DEBUG_PRINT(debug, "."); // 设备状态回调
        // 内置状态处理
        status_change("0_ing_after");
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing_after");
        }
        delay(250);
    }
    // 连接失败并且没有存储wifi信息就重启板子
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN(debug, ("设置 WIFI 连接失败, 即将重启设备"));
        ESP.restart();
        return false;
    }
    DEBUG_PRINTLN(debug, "");
    DEBUG_PRINT(debug, F("IP address: "));
    DEBUG_PRINTLN(debug, WiFi.localIP());

    for (int i = 0; i < keys.length(); i++)
    {
        String key = keys[i];
        JSONVar value = data[key];
        set_local_data(key, String((const char *)value));
    }
    delay(250);
    DEBUG_PRINTLN(debug, ("WIFI 连接成功, 即将重启设备更新设置。"));
    ESP.restart();

    DEBUG_PRINTLN(debug, ("==============================================="));

    return true;
}
