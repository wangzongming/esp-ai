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

/**
 * 聆听中时不需要被打断，否则会带来更多问题
 * 比如：不做回音消除时用户说话就不可以触发唤醒词等等...
 */
void ESP_AI::on_wakeup()
{
    long esp_ai_last_debounce_time = 0;
    int esp_ai_debounce_delay = 200;
    int esp_ai_prev_state = 0;
    int esp_ai_prev_state_listen = 0;
    long esp_ai_btn_up_time = 0;

    while (true)
    {
        if ((wake_up_scheme == "pin_high" || wake_up_scheme == "pin_low"))
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
                        esp_ai_prev_state = reading;
                        DEBUG_PRINTLN(debug, ("[Info] -> 按下了按钮唤醒"));
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
        // else if ((asr_ing == false && esp_ai_start_ed == "0" && (wake_up_scheme == "asrpro" || wake_up_scheme == "serial")))
        else if ((asr_ing == false && (wake_up_scheme == "asrpro" || wake_up_scheme == "serial")))
        {
            if (Esp_ai_serial.available())
            {
                String command = Esp_ai_serial.readStringUntil('\n');
                String clear_str = cleanString(command);
                if (clear_str == String(wake_up_config.str))
                {
                    DEBUG_PRINTLN(debug, ("[Info] -> 收到串口唤醒"));
                    wakeUp("wakeup");
                }
            }
            else if (Serial.available())
            {
                String command = Serial.readStringUntil('\n');
                String clear_str = cleanString(command);
                if (clear_str == String(wake_up_config.str))
                {
                    DEBUG_PRINTLN(debug, ("[Info] -> 收到串口唤醒"));
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
                if (esp_ai_start_send_audio)
                {
                    if (esp_ai_btn_up_time == 0)
                    {
                        esp_ai_btn_up_time = millis();
                    }
                    if ((millis() - esp_ai_btn_up_time) >= 300)
                    {

                        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                        {
                            DEBUG_PRINTLN(debug, ("[Info] -> 大语言模型正在推理。"));
                            esp_ai_webSocket.sendTXT("{\"type\":\"iat_end\"}");
 
                            xSemaphoreGive(esp_ai_ws_mutex);
                        }
                    }
                }
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}