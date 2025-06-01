/*
 * MIT License
 *
 * Copyright (c) 2025-至今 小明IO
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
        wait_mp3_player_done();
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

    DEBUG_PRINTLN(debug, ("WIFI 连接成功, 即将重启设备更新设置。"));
    wait_mp3_player_done();
    ESP.restart();

    DEBUG_PRINTLN(debug, ("==============================================="));

    return true;
}
