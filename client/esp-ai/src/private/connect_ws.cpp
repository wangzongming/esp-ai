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
#include "connect_ws.h"
 
void ESP_AI::connect_ws()
{
    if (String(server_config.protocol) != "https" && String(server_config.protocol) != "http")
    {
        DEBUG_PRINTLN(debug, F("[Error] 服务协议必须为 http 或者 https ！"));
        DEBUG_PRINTLN(debug, F("[Error] 正确的服务配置： ESP_AI_server_config server_config = { \"http或者https\", \"ip或者域名\", 端口 };"));
        return;
    }
    String loc_device_id = get_device_id(); 
    String loc_api_key = get_local_data("api_key");
    String loc_ext1 = get_local_data("ext1");
    String loc_ext2 = get_local_data("ext2");
    String loc_ext3 = get_local_data("ext3");
    String loc_ext4 = get_local_data("ext4");
    String loc_ext5 = get_local_data("ext5");
    String loc_ext6 = get_local_data("ext6");
    String loc_ext7 = get_local_data("ext7");

    DEBUG_PRINTLN(debug, ("[Info] 开始连接后台服务，如果长时间无响应说明你的服务有问题。"));
    DEBUG_PRINT(debug, ("[Info] 主机："));
    DEBUG_PRINTLN(debug, server_config.ip);
    DEBUG_PRINT(debug, ("[Info] 协议："));
    DEBUG_PRINTLN(debug, server_config.protocol);
    DEBUG_PRINT(debug, ("[Info] 端口："));
    DEBUG_PRINTLN(debug, server_config.port);
    DEBUG_PRINT(debug, ("[Info] 路径："));
    DEBUG_PRINTLN(debug, server_config.path);
    DEBUG_PRINT(debug, ("[Info] 参数："));
    DEBUG_PRINTLN(debug, server_config.params);

    // size_t freeHeap = ESP.getFreeHeap() / 1024;
    // Serial.print("剩余内存：");
    // Serial.println(freeHeap);


    // ws 服务
    if (String(server_config.protocol) == "https")
    {
        esp_ai_webSocket.beginSSL(
            server_config.ip,
            server_config.port,
            String(server_config.path) + "/?v=" + ESP_AI_VERSION +
                "&device_id=" + loc_device_id +
                "&api_key=" + loc_api_key +
                "&ext1=" + loc_ext1 +
                "&ext2=" + loc_ext2 +
                "&ext3=" + loc_ext3 +
                "&ext4=" + loc_ext4 +
                "&ext5=" + loc_ext5 +
                "&ext6=" + loc_ext6 +
                "&ext7=" + loc_ext7 +
                "&" + server_config.params);
    }
    else
    {
        esp_ai_webSocket.begin(
            server_config.ip,
            server_config.port,
            String(server_config.path) + "/?v=" + ESP_AI_VERSION +
                "&device_id=" + loc_device_id +
                "&api_key=" + loc_api_key +
                "&ext1=" + loc_ext1 +
                "&ext2=" + loc_ext2 +
                "&ext3=" + loc_ext3 +
                "&ext4=" + loc_ext4 +
                "&ext5=" + loc_ext5 +
                "&ext6=" + loc_ext6 +
                "&ext7=" + loc_ext7 +
                "&" + server_config.params);
    }
  
    esp_ai_webSocket.onEvent(std::bind(&ESP_AI::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    esp_ai_webSocket.setReconnectInterval(3000);
    esp_ai_webSocket.enableHeartbeat(5000, 10000, 0); 
}