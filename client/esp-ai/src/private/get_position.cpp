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

void ESP_AI::get_position()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }
    // 创建一个HTTP客户端对象
    HTTPClient http;

    String ip = "";
    String nation = "";
    String province = "";
    String city = "";

    DEBUG_PRINTLN(debug, "[Info] 定位中...");
    /********  接口1： ***********/
    String api1 = "https://webapi-pc.meitu.com/common/ip_location";

    http.begin(api1);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        Serial.printf("[HTTPS] GET code: %d\n", httpCode);
        Serial.printf("[HTTPS] GET res: %d\n", payload);
        JSONVar parse_res = JSON.parse(payload);
        if (parse_res.hasOwnProperty("data"))
        {
            for (int x = 0; x < parse_res["data"].keys().length(); x++)
            {
                ip = (const char *)parse_res["data"].keys()[x];
                nation = (const char *)parse_res["data"][parse_res["data"].keys()[x]]["nation"];
                province = (const char *)parse_res["data"][parse_res["data"].keys()[x]]["province"];
                city = (const char *)parse_res["data"][parse_res["data"].keys()[x]]["city"];
            }

            DEBUG_PRINT(debug, "[Info] 定位完毕：");
            DEBUG_PRINT(debug, nation);
            DEBUG_PRINT(debug, "/");
            DEBUG_PRINT(debug, province);
            DEBUG_PRINT(debug, "/");
            DEBUG_PRINTLN(debug, city);

            if (onPositionCb != nullptr)
            {
                onPositionCb(ip, nation, province, city);
            }
            return;
        }
        else
        {
            DEBUG_PRINTLN(debug, "[Info] 定位接口1请求失败， 返回数据错误，即将使用接口2进行...");
            http.end();
        }
    }
    else
    {
        DEBUG_PRINTLN(debug, "[Info] 定位接口1请求失败，即将使用接口2进行...");
        http.end();
    }

    /********  接口2： ***********/
    String api2 = "https://g3.letv.com/r?format=1"; 
    http.begin(api2);
    http.addHeader("Content-Type", "application/json");
    int httpCode2 = http.GET();

    if (httpCode2 > 0)
    {
        String payload = http.getString();
        Serial.printf("[HTTPS] GET code: %d\n", httpCode2);
        Serial.printf("[HTTPS] GET res: %d\n", payload);
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

            if (onPositionCb != nullptr)
            {
                onPositionCb(ip, nation, province, city);
            }
            return;
        }
        else
        {
            DEBUG_PRINTLN(debug, "[Info] 定位接口2请求失败， 返回数据错误。");
            http.end();
        }
    }
    else
    {
        Serial.printf("[HTTPS] GET code: %d\n", httpCode2);
        DEBUG_PRINTLN(debug, "[Info] 定位接口2请求失败");
        http.end();
    }

  
}