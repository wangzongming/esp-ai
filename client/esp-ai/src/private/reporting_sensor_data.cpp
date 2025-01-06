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
#include "reporting_sensor_data.h"

JSONVar digitalReadJSONData;
JSONVar analogReadJSONData;

void ESP_AI::reporting_sensor_data_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->reporting_sensor_data();
}

void ESP_AI::reporting_sensor_data()
{
    while (true)
    {
        for (int i = 0; i < digital_read_pins.size(); i++)
        {
            int pin = digital_read_pins[i];
            int reading = digitalRead(pin);
            digitalReadJSONData["type"] = "digitalRead";
            digitalReadJSONData["pin"] = pin;
            digitalReadJSONData["value"] = reading;
            String sendData = JSON.stringify(digitalReadJSONData);
            esp_ai_webSocket.sendTXT(sendData);
        }
        for (int i = 0; i < analog_read_pins.size(); i++)
        {
            int pin = analog_read_pins[i];
            int reading = analogRead(pin);
            analogReadJSONData["type"] = "analogRead";
            analogReadJSONData["pin"] = pin;
            analogReadJSONData["value"] = reading;
            String sendData = JSON.stringify(analogReadJSONData);
            esp_ai_webSocket.sendTXT(sendData);
        }
 
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}