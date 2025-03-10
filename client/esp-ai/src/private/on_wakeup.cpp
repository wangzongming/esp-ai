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
#include "on_wakeup.h"
String cleanString(String input)
{
    String output = "";
    for (int i = 0; i < input.length(); i++)
    {
        char c = input.charAt(i);
        if (c != '\n' && c != '\r')
        {                // 如果字符不是换行符或回车符
            output += c; // 将其添加到输出字符串
        }
    }
    return output;
}

void ESP_AI::on_wakeup_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->on_wakeup();
}

void ESP_AI::on_wakeup()
{
    long esp_ai_last_debounce_time = 0;
    int esp_ai_debounce_delay = 200;
    int esp_ai_prev_state = 0;
    int esp_ai_prev_state_listen = 0;
    long esp_ai_btn_up_time = 0;
 
    while (true)
    {
        if (wake_up_scheme == "pin_high" || wake_up_scheme == "pin_low")
        {
            int reading = digitalRead(wake_up_config.pin);
            long curTime = millis();
            int target_val = wake_up_scheme == "pin_high" ? 1 : 0;
            if (reading == target_val)
            {
                if ((curTime - esp_ai_last_debounce_time) > esp_ai_debounce_delay)
                {
                    esp_ai_last_debounce_time = curTime;
                    if (esp_ai_prev_state != reading)
                    {
                        esp_ai_start_send_audio = false;
                        esp_ai_start_get_audio = false;
                        esp_ai_prev_state = reading;
                        DEBUG_PRINTLN(debug, ("按下了按钮, 唤醒成功"));
                        wakeUp("wakeup");
                    }
                }
            }
            else
            {
                esp_ai_prev_state = reading;
            }
        }
        // else if ((esp_ai_start_ed == "0" && (wake_up_scheme == "asrpro" || wake_up_scheme == "serial")))
        else if ((asr_ing == false && (wake_up_scheme == "asrpro" || wake_up_scheme == "serial")))
        {
            if (Esp_ai_serial.available())
            {
                String command = Esp_ai_serial.readStringUntil('\n');
                String clear_str = cleanString(command);
                if (clear_str == String(wake_up_config.str))
                {
                    esp_ai_start_send_audio = false;
                    esp_ai_start_get_audio = false;
                    DEBUG_PRINTLN(debug, ("[Info] -> 收到串口数据, 唤醒成功"));
                    wakeUp("wakeup");
                }
            }
            else if (Serial.available())
            {
                String command = Serial.readStringUntil('\n');
                String clear_str = cleanString(command);
                if (clear_str == String(wake_up_config.str))
                {
                    esp_ai_start_send_audio = false;
                    esp_ai_start_get_audio = false;
                    DEBUG_PRINTLN(debug, ("[Info] -> 收到串口数据, 唤醒成功"));
                    wakeUp("wakeup");
                }
            }
        }
        else if (esp_ai_is_listen_model)
        {
            int reading = digitalRead(wake_up_config.pin);
            int target_val = wake_up_scheme == "pin_high_listen" ? 1 : 0;
            if ((reading == target_val))
            {
                if (esp_ai_prev_state_listen != reading)
                {
                    esp_ai_start_send_audio = false;
                    esp_ai_start_get_audio = false;
                    esp_ai_btn_up_time = 0;
                    esp_ai_prev_state_listen = reading;
                    DEBUG_PRINTLN(debug, ("[Info] -> 您请说话。"));
                    wakeUp("wakeup");
                }
            }
            else
            {
                // 需要给 mic 和解码器一些时间来处理还没有收集到的数据
                esp_ai_prev_state_listen = reading;
                if (esp_ai_start_get_audio)
                {
                    if (esp_ai_btn_up_time == 0)
                    {
                        esp_ai_btn_up_time = millis();
                    }
                    if ((millis() - esp_ai_btn_up_time) >= 300) 
                    { 
                        DEBUG_PRINTLN(debug, ("[Info] -> 大语言模型正在推理。"));
                        esp_ai_start_get_audio = false;
                        esp_ai_start_send_audio = false;
                        esp_ai_webSocket.sendTXT("{\"type\":\"iat_end\"}");
                    } 
                }
            }
 
        }
 
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}