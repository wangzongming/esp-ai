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