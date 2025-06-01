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

            if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                int pin = digital_read_pins[i];
                int reading = digitalRead(pin);
                digitalReadJSONData["type"] = "digitalRead";
                digitalReadJSONData["pin"] = pin;
                digitalReadJSONData["value"] = reading;
                String sendData = JSON.stringify(digitalReadJSONData);
                esp_ai_webSocket.sendTXT(sendData);
                xSemaphoreGive(esp_ai_ws_mutex);
            }
        }
        for (int i = 0; i < analog_read_pins.size(); i++)
        {
            if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                int pin = analog_read_pins[i];
                int reading = analogRead(pin);
                analogReadJSONData["type"] = "analogRead";
                analogReadJSONData["pin"] = pin;
                analogReadJSONData["value"] = reading;
                String sendData = JSON.stringify(analogReadJSONData);
                esp_ai_webSocket.sendTXT(sendData);
                xSemaphoreGive(esp_ai_ws_mutex);
            } 
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}