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
#include "open_ap.h"

void ESP_AI::open_ap()
{ 
    String ap_name = get_ap_name(wifi_config.ap_name);
 
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_name.c_str());
    IPAddress ip = WiFi.softAPIP();
    String httpUrl = String("http://") + ip.toString();
    DEBUG_PRINTLN(debug, "[Info] WIFI名称：" + ap_name);
    DEBUG_PRINTLN(debug, "[Info] 配网地址：" + httpUrl);

    if (esp_ai_dns_server.start(53, "*", ip))
    { 
        DEBUG_PRINTLN(debug, "[Info] DNS 服务器启动成功");
    }
    else
    {
        DEBUG_PRINTLN(debug, "[Error] DNS 服务器启动失败"); 
    }

    play_builtin_audio(qing_pei_wang, qing_pei_wang_len);

    xTaskCreate(ESP_AI::scan_wifi_wrapper, "scan_wifi", 1024 * 8, this, 1, NULL);

    // 启动配网服务
    web_server_init();
    if (onAPInfoCb != nullptr)
    {
        onAPInfoCb(httpUrl, ip.toString(), ap_name);
    }
    // 内置状态处理
    status_change("0_ap");
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "0_ap";
        onNetStatusCb("0_ap");
    }
}