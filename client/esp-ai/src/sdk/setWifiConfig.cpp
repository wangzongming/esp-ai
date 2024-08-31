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
bool ESP_AI::setWifiConfig(char wifi_name[60], char wifi_pwd[60], char api_key[60], char ext1[60], char ext2[60])
{
    WiFi.disconnect();
    // 清除之前的连接状态, 给 Wi-Fi 模块一些时间来断开连接
    WiFi.mode(WIFI_OFF);
    delay(1000);

    strncpy(wifi_config.wifi_name, wifi_name, sizeof(wifi_config.wifi_name) - 1);
    wifi_config.wifi_name[sizeof(wifi_config.wifi_name) - 1] = '\0';
    strncpy(wifi_config.wifi_pwd, wifi_pwd, sizeof(wifi_config.wifi_pwd) - 1);
    wifi_config.wifi_pwd[sizeof(wifi_config.wifi_pwd) - 1] = '\0';

    DEBUG_PRINTLN(debug, "==================== 调用方法设置信息 ====================");
    DEBUG_PRINTLN(debug, "wifi name: " + String(wifi_config.wifi_name));
    DEBUG_PRINTLN(debug, "wifi pwd: " + String(wifi_config.wifi_pwd));
    WiFi.begin(wifi_config.wifi_name, wifi_config.wifi_pwd);
    DEBUG_PRINT(debug, "connect wifi ing..");

    int connect_count = 0;
    // 10s 连不上Wifi的话
    int try_count = 20;
    while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
    {
        connect_count++;
        delay(500);
        DEBUG_PRINT(debug, ".");
    }
    // 连接失败并且没有存储wifi信息就重启板子
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN(debug, "设置 WIF 连接失败");
        String loc_wifi_name = get_local_data("wifi_name");
        String loc_wifi_pwd = get_local_data("wifi_pwd");
        WiFi.begin(loc_wifi_name, loc_wifi_pwd);
        int connect_count = 0;
        // 连不上就重启板子,让 begin 进行热点打开
        int try_count = 20;
        while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
        {
            // 设备状态回调
            if (onNetStatusCb != nullptr)
            {
                net_status = "0_ing";
                onNetStatusCb("0_ing");
            }
            
            connect_count++;
            delay(500);
            DEBUG_PRINT(debug, ".");
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            DEBUG_PRINTLN(debug, "设置 WIFI 连接失败，重启板子");
            ESP.restart();
        }
        return false;
    }
    DEBUG_PRINTLN(debug, "");
    DEBUG_PRINT(debug, "IP address: ");
    DEBUG_PRINTLN(debug, WiFi.localIP());
    DEBUG_PRINTLN(debug, "===============================================");
    // 持久保存， 这一步都是可选的
    if (String(wifi_name).length() > 0)
    {
        set_local_data("wifi_name", String(wifi_name));
    }
    if (String(wifi_pwd).length() > 0)
    {
        set_local_data("wifi_pwd", String(wifi_pwd));
    }
    if (String(api_key).length() > 0)
    {
        set_local_data("api_key", String(api_key));
    }
    if (String(ext1).length() > 0)
    {
        set_local_data("ext1", String(ext1));
    }
    if (String(ext2).length() > 0)
    {
        set_local_data("ext2", String(ext2));
    }
    return true;
}
