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
#include "get_server_config.h"
#include <HTTPClient.h>

// 获取服务节点信息
bool ESP_AI::get_server_config()
{
    // 创建一个HTTP客户端对象
    HTTPClient http;

    String loc_api_key = get_local_data("api_key");
    if (loc_api_key == "")
    {
        DEBUG_PRINTLN(debug, ("\n[Info] api_key 为空，需要自行在客户端代码中配置【服务IP与端口】。 "));
        DEBUG_PRINTLN(debug, ("[Info] 如果您已经配置了，那请忽略本提示。"));
        DEBUG_PRINTLN(debug, ("[Info] 在配网页面中填入 ESP-AI 开放平台中的超体 api_key 即可完成服务对接。开放平台：https://dev.espai.fun\n"));
        return true;
    }
    DEBUG_PRINTLN(debug, "[Info] api_key：" + loc_api_key);
    String domain = "https://api.espai.fun";
    String url = domain + "/sdk/get_server_info_by_api_key?api_key=" + loc_api_key;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // Serial.println("payload: ");
        // Serial.println(payload);
        JSONVar parse_res = JSON.parse(payload);
        if (JSON.typeof(parse_res) == "undefined")
        {
            return false;
        }

        if (parse_res.hasOwnProperty("success"))
        {
            bool success = (bool)parse_res["success"];
            String message = (const char *)parse_res["message"];
            if (success == false)
            {
                if (onErrorCb != nullptr)
                {
                    onErrorCb("003", "", "");
                }
                Serial.println("[Error] 请求服务信息错误: " + message);
                Serial.println("[Error] 请重新配网，如果您不使用 ESP-AI 开发者平台，请将配网页面中的 api_key 删除");
                open_ap(); 
                return false;
            }
            else
            {
                String ip_str = (const char *)parse_res["data"]["ip"];
                // String ip_str = "192.168.3.3";
                int port = (int)parse_res["data"]["port"];
                String protocol = (const char *)parse_res["data"]["protocol"];
                String path = (const char *)parse_res["data"]["path"];

                DEBUG_PRINTLN(debug, "[Info] 服务协议: " + protocol);
                DEBUG_PRINTLN(debug, "[Info] 服务IP: " + ip_str);
                DEBUG_PRINTLN(debug, "[Info] 服务端口: " + String(port));

                strcpy(server_config.ip, ip_str.c_str());
                strcpy(server_config.protocol, protocol.c_str());
                strcpy(server_config.path, path.c_str());
                server_config.port = port;
                return true;
            }
        }
    }
    else
    {
        Serial.printf("\n[HTTPS] 获取ESP-AI服务节点信息失败, error: %s\n", http.errorToString(httpCode).c_str());
        return false;
    }

    http.end();
}