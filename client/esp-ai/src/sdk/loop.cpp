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
 * 源码已经是免费使用了，如果您连版权都不保留您认为这属于什么行为？
 *
 * @author 小明IO
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */
#include "loop.h"

void ESP_AI::loop()
{
    if (esp_ai_status == "0_ap" && wifi_config.way == "AP")
    {
        esp_ai_dns_server.processNextRequest();
    }

    esp_ai_server.handleClient();
    esp_ai_webSocket.loop();

    if (WiFi.status() != WL_CONNECTED)
    {

        // DEBUG_PRINTLN(debug, F("WiFi 断开，尝试重连..."));
        bool status_0 = esp_ai_net_status == "2" && esp_ai_net_status != "0" && ap_connect_err != "1";
        if (status_0)
        {
            // 内置状态处理
            status_change("0");

            // 设备状态回调
            if (onNetStatusCb != nullptr)
            {
                esp_ai_net_status = "0";
                onNetStatusCb("0");
                DEBUG_PRINTLN(debug, ("[Error] -> WIFI 异常断开，将自动重启板子"));
                ESP.restart();
            }
        }

        return;
    }
}