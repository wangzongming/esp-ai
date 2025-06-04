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
#include "get_server_config.h"

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
    String domain = "https://api.espai2.fun";
    String url = domain + "/sdk/get_server_info_by_api_key?api_key=" + loc_api_key;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode); 
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
                if (wifi_config.way == "BLE")
                {
                    open_ble_server();
                }
                else
                {
                    open_ap();
                }
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