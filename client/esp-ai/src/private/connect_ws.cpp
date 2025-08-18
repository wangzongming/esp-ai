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


    // size_t freeHeap = ESP.getFreeHeap() / 1024;
    // Serial.print("剩余内存：");
    // Serial.println(freeHeap);

    String real_path = String(server_config.path) + "/?v=" + ESP_AI_VERSION + "&device_id=" + loc_device_id;
    if (loc_api_key)
    {
        real_path += ("&api_key=" + loc_api_key);
    }
    if (loc_ext1 || loc_api_key)
    {
        real_path += ("&ext1=" + (loc_ext1 ? loc_ext1 : loc_api_key));
    }
    if (loc_ext2 != "")
    {
        real_path += ("&ext2=" + loc_ext2);
    }
    if (loc_ext3 != "")
    {
        real_path += ("&ext3=" + loc_ext3);
    }
    if (loc_ext4 != "")
    {
        real_path += ("&ext4=" + loc_ext4);
    }
    if (loc_ext5 != "")
    {
        real_path += ("&ext5=" + loc_ext5);
    }
    if (loc_ext6 != "")
    {
        real_path += ("&ext6=" + loc_ext6);
    }
    if (loc_ext7 != "")
    {
        real_path += ("&ext7=" + loc_ext7);
    }
#if defined(AUDIO_OUTPUT_BITRATE)
    real_path += ("&bitrate=" + String(AUDIO_OUTPUT_BITRATE));
#endif

#if defined(LITTLE_ROM)
    real_path += ("&LITTLE_ROM=1");
#endif 
    // 增加设备缓存能力参数
    real_path += ("&AUDIO_BUFFER_SIZE=" + String(AUDIO_BUFFER_SIZE));
    if (server_config.params)
    {
        real_path += ("&" + String(server_config.params));
    }
    DEBUG_PRINTLN(debug, F("[Info] 开始连接后台服务，如果长时间无响应说明你的服务有问题。"));
    DEBUG_PRINT(debug, server_config.protocol);
    DEBUG_PRINT(debug, " ");
    DEBUG_PRINT(debug, server_config.ip);
    DEBUG_PRINT(debug, " ");
    DEBUG_PRINT(debug, server_config.port);
    DEBUG_PRINT(debug, " ");
    DEBUG_PRINT(debug, server_config.path);
    DEBUG_PRINT(debug, " ");
    DEBUG_PRINT(debug, server_config.params); 
    DEBUG_PRINT(debug, " ");
    DEBUG_PRINTLN(debug, real_path); 

#if defined(LITTLE_ROM)
    if (String(server_config.protocol) == "https")
    {
        DEBUG_PRINTLN(debug, F("[Error] 此开发板不支持 https 连接 ！"));
    }
    esp_ai_webSocket.begin(server_config.ip, server_config.port, real_path);
#else
    if (String(server_config.protocol) == "https")
    {
        esp_ai_webSocket.beginSSL(server_config.ip, server_config.port, real_path);
    }
    else
    {
        esp_ai_webSocket.begin(server_config.ip, server_config.port, real_path);
    }
#endif

    esp_ai_webSocket.onEvent(std::bind(&ESP_AI::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    esp_ai_webSocket.setReconnectInterval(3000);
    esp_ai_webSocket.enableHeartbeat(5000, 10000, 0);
}