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
#include "get_position.h"
#include <HTTPClient.h>


void ESP_AI::get_position_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->get_position();
}

HTTPClient esp_ai_get_position_http; 
void ESP_AI::get_position()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }
 
    String ip = "";
    String nation = "";
    String province = "";
    String city = "";

    DEBUG_PRINTLN(debug, "[Info] 定位中..."); 
    String api2 = "https://g3.letv.com/r?format=1";
    esp_ai_get_position_http.begin(api2);
    esp_ai_get_position_http.addHeader("Content-Type", "application/json");
    esp_ai_get_position_http.setTimeout(10000);   
    int httpCode2 = esp_ai_get_position_http.GET();
  
    if (httpCode2 > 0)
    {
        String payload = esp_ai_get_position_http.getString();
        Serial.printf("[HTTPS] GET code: %d\n", httpCode2); 
        JSONVar parse_res = JSON.parse(payload);
        if (parse_res.hasOwnProperty("desc"))
        {

            String address = (const char *)parse_res["desc"];

            int index = 0;
            String parts[4];
            int pos = address.indexOf("-");
            while (pos != -1 && index < 4)
            {
                parts[index++] = address.substring(0, pos);
                address = address.substring(pos + 1);
                pos = address.indexOf("-");
            }

            ip = (const char *)parse_res["host"];
            nation = parts[0];
            province = parts[1];
            city = parts[2];  
            DEBUG_PRINT(debug, "[Info] 定位完毕：");
            DEBUG_PRINT(debug, nation);
            DEBUG_PRINT(debug, "/");
            DEBUG_PRINT(debug, province);
            DEBUG_PRINT(debug, "/");
            DEBUG_PRINTLN(debug, city);  
            esp_ai_get_position_http.end(); 
 
            delay(100);
            if (onPositionCb != nullptr)
            {
                onPositionCb(ip, nation, province, city);
            }
            // 删除任务
            vTaskDelete(NULL);
            return;
        }
        else
        {
            DEBUG_PRINTLN(debug, "[Info] 定位请求失败， 返回数据错误。");
            esp_ai_get_position_http.end();
        }
    }
    else
    {
        Serial.printf("[HTTPS] GET code: %d\n", httpCode2);
        DEBUG_PRINTLN(debug, "[Info] 定位请求失败");
        esp_ai_get_position_http.end();
    }
 
    vTaskDelete(NULL);
}