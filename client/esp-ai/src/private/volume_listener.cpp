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
#include "volume_listener.h"
void ESP_AI::volume_listener_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->volume_listener();
}

void ESP_AI::volume_listener()
{ 
    const int numReadings = 20;  
    int readings[numReadings];
    int readIndex = 0;
    int total = 0;
    int average = 0;

    for (int i = 0; i < numReadings; i++)
    {
        readings[i] = 0;
    }

    while (true)
    {
        total -= readings[readIndex];
        readings[readIndex] = analogRead(volume_config.input_pin);
        total += readings[readIndex];
        readIndex = (readIndex + 1) % numReadings;
        average = total / numReadings;
        float _t = static_cast<float>(average) / volume_config.max_val;
        String formattedValue = String(_t, 1);
        float formattedNumber = formattedValue.toFloat();
        if (fabs(volume_config.volume - formattedNumber) >= 0.1)
        { 
            volume_config.volume = formattedNumber;
            esp_ai_volume.setVolume(volume_config.volume);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}