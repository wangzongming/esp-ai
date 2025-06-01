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
#include "get_position.h"


void ESP_AI::get_position_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->get_position();
}

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
    String latitude = "";
    String longitude = "";

    DEBUG_PRINTLN(debug, "[Info] 定位中..."); 

    String api2 = "http://api.espai2.fun/sdk/position";
    HTTPClient esp_ai_get_position_http; 
    esp_ai_get_position_http.begin(api2);
    esp_ai_get_position_http.addHeader("Content-Type", "application/json");
    esp_ai_get_position_http.setTimeout(10000);   
    int httpCode2 = esp_ai_get_position_http.GET();
  
    if (httpCode2 > 0)
    {
        String payload = esp_ai_get_position_http.getString();
        Serial.printf("[HTTPS] GET code: %d\n", httpCode2); 
        JSONVar parse_res = JSON.parse(payload);
        if (parse_res.hasOwnProperty("success"))
        { 

            ip = (const char *)parse_res["data"]["ip"];
            nation = (const char *)parse_res["data"]["country_name"];
            province = (const char *)parse_res["data"]["region"];
            city = (const char *)parse_res["data"]["city"];
            latitude = (const char *)parse_res["data"]["latitude"];
            longitude = (const char *)parse_res["data"]["longitude"]; 
            DEBUG_PRINT(debug, "[Info] 定位完毕：");
            DEBUG_PRINT(debug, nation);
            DEBUG_PRINT(debug, "/");
            DEBUG_PRINT(debug, province);
            DEBUG_PRINT(debug, "/");
            DEBUG_PRINTLN(debug, city);  
            esp_ai_get_position_http.end(); 
            if (onPositionCb != nullptr)
            {
                onPositionCb(ip, nation, province, city, latitude, longitude);
            }
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

    // 任务执行完毕，删除自身 
    get_position_task_handle = NULL;  
    vTaskDelete(NULL);
}