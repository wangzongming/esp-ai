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
#include "open_ap.h"
 
void ESP_AI::open_ap()
{
    WiFi.mode(WIFI_AP);
    String ap_name = strlen(wifi_config.ap_name) > 0 ? wifi_config.ap_name : "ESP-AI";
    WiFi.softAP(ap_name);
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    String httpUrl = "http://" + ipStr;
    DEBUG_PRINTLN(debug, "[Info] WIFI名称：" + ap_name);
    DEBUG_PRINTLN(debug, "[Info] 配网地址：" + httpUrl);
    
    esp_ai_dec.write(qing_pei_wang, qing_pei_wang_len);
 
    xTaskCreate(ESP_AI::scan_wifi_wrapper, "scan_wifi", 1024 * 8, this, 1, NULL);

    // 启动配网服务
    web_server_init();
    if (onAPInfoCb != nullptr)
    {
        onAPInfoCb(httpUrl, ipStr, ap_name);
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