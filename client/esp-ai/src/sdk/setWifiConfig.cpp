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
bool ESP_AI::setWifiConfig(String wifi_name, String wifi_pwd, String api_key, String ext1, String ext2, String ext3, String ext4, String ext5, String ext6, String ext7)
{
    DEBUG_PRINTLN(debug, ("==================== 调用 setWifiConfig 方法设置信息 ===================="));

    WiFi.begin(wifi_name, wifi_pwd);
    DEBUG_PRINT(debug, "connect wifi ing..");

    // WiFi.disconnect();
    // // 清除之前的连接状态, 给 Wi-Fi 模块一些时间来断开连接
    // WiFi.mode(WIFI_OFF);
    // delay(1000);

    // strncpy(wifi_config.wifi_name, wifi_name, sizeof(wifi_config.wifi_name) - 1);
    // wifi_config.wifi_name[sizeof(wifi_config.wifi_name) - 1] = '\0';
    // strncpy(wifi_config.wifi_pwd, wifi_pwd, sizeof(wifi_config.wifi_pwd) - 1);
    // wifi_config.wifi_pwd[sizeof(wifi_config.wifi_pwd) - 1] = '\0';

    // DEBUG_PRINTLN(debug, "wifi name: " + String(wifi_config.wifi_name));
    // DEBUG_PRINTLN(debug, "wifi pwd: " + String(wifi_config.wifi_pwd));
    // WiFi.begin(wifi_config.wifi_name, wifi_config.wifi_pwd);
    // DEBUG_PRINT(debug, "connect wifi ing..");

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
            net_status = "0_ing";
            onNetStatusCb("0_ing");
        }
        delay(250);
        DEBUG_PRINT(debug, "."); // 设备状态回调
        // 内置状态处理
        status_change("0_ing_after");
        if (onNetStatusCb != nullptr)
        {
            net_status = "0_ing";
            onNetStatusCb("0_ing_after");
        }
        delay(250);
    }
    // 连接失败并且没有存储wifi信息就重启板子
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN(debug, ("设置 WIFI 连接失败, 即将重启板子"));
        ESP.restart();

        // 上一个网络依旧存在连不上的概率，所以这里没必要这么设置...
        // String loc_wifi_name = get_local_data("wifi_name");
        // String loc_wifi_pwd = get_local_data("wifi_pwd");
        // WiFi.begin(loc_wifi_name, loc_wifi_pwd);
        // int connect_count = 0;
        // // 连不上就重启板子,让 begin 进行热点打开
        // int try_count = 15;
        // while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
        // {
        //     // 设备状态回调
        //     if (onNetStatusCb != nullptr)
        //     {
        //         net_status = "0_ing";
        //         onNetStatusCb("0_ing");
        //     }
        //     connect_count++;
        //     delay(250);
        //     DEBUG_PRINT(debug, ".");
        //     if (onNetStatusCb != nullptr)
        //     {
        //         net_status = "0_ing";
        //         onNetStatusCb("0_ing_after");
        //     }
        //     delay(250);
        // }
        // if (WiFi.status() != WL_CONNECTED)
        // {
        //     DEBUG_PRINTLN(debug, ("设置 WIFI 连接失败，即将重启板子"));
        //     ESP.restart();
        // }
        return false;
    }
    DEBUG_PRINTLN(debug, "");
    DEBUG_PRINT(debug, F("IP address: "));
    DEBUG_PRINTLN(debug, WiFi.localIP());
    DEBUG_PRINTLN(debug, ("==============================================="));
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
    if (String(ext3).length() > 0)
    {
        set_local_data("ext3", String(ext3));
    }
    if (String(ext4).length() > 0)
    {
        set_local_data("ext4", String(ext4));
    }
    if (String(ext5).length() > 0)
    {
        set_local_data("ext5", String(ext5));
    }
    if (String(ext6).length() > 0)
    {
        set_local_data("ext6", String(ext6));
    }
    if (String(ext7).length() > 0)
    {
        set_local_data("ext7", String(ext7));
    }
    return true;
}
