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
 
StaticTask_t onWakeupTaskBuffer;
StackType_t onWakeupTaskStack[ON_WAKE_UP_TASK_SIZE];
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

/**
 * 聆听中时不需要被打断，否则会带来更多问题
 * 比如：不做回音消除时用户说话就不可以触发唤醒词等等...
 */
void on_wakeup_task_static(void *arg)
{
    OnWakeUpContext *ctx = static_cast<OnWakeUpContext *>(arg);
    long last_debounce_time = 0;
    int debounce_delay = 150;
    int prev_pin_state = 0;
    int prev_listen_state = 0;
    long btn_up_time = 0;

    while (true)
    {
        long curTime = millis();

        // 方式一：GPIO 触发
        if (*ctx->wake_up_scheme == "pin_high" || *ctx->wake_up_scheme == "pin_low")
        {
            int target_val = (*ctx->wake_up_scheme == "pin_high") ? HIGH : LOW;
            int reading = digitalRead(*ctx->pin);

            if (reading == target_val)
            {
                if ((curTime - last_debounce_time) > debounce_delay && prev_pin_state != reading)
                {
                    last_debounce_time = curTime;
                    prev_pin_state = reading;
                    DEBUG_PRINTLN(ctx->debug, F("[Info] -> 按钮唤醒触发"));
                    ctx->wakeUp("wakeup");
                }
            }
            else
            {
                prev_pin_state = reading;
            }
        }

        // 方式二：串口唤醒（非 ASR 时）
        else if (!(*ctx->asr_ing) && (*ctx->wake_up_scheme == "asrpro" || *ctx->wake_up_scheme == "serial"))
        {
            if (ctx->Esp_ai_serial->available())
            {
                String command = ctx->Esp_ai_serial->readStringUntil('\n');
                String cleaned = cleanString(command); 
                if (cleaned == ctx->wake_up_str)
                {
                    DEBUG_PRINTLN(ctx->debug, F("[Info] -> 串口唤醒触发"));
                    ctx->wakeUp("wakeup");
                }
            }
        }

        // 方式三：监听模式，按钮控制
        else if (*ctx->esp_ai_is_listen_model)
        {
            int target_val = (*ctx->wake_up_scheme == "pin_high_listen") ? HIGH : LOW;
            int reading = digitalRead(*ctx->pin);

            if (reading == target_val && prev_listen_state != reading)
            {
                btn_up_time = 0;
                prev_listen_state = reading;
                DEBUG_PRINTLN(ctx->debug, F("[Info] -> 您请说话。"));
                ctx->wakeUp("wakeup");
            }
            else if (reading != target_val)
            {
                prev_listen_state = reading;

                if (*(ctx->esp_ai_start_send_audio))
                {
                    if (btn_up_time == 0)
                        btn_up_time = millis();

                    if ((curTime - btn_up_time) >= 300)
                    {

                        *ctx->esp_ai_start_send_audio = false;
                        DEBUG_PRINTLN(ctx->debug, F("[Info] -> 大语言模型正在推理。"));
                        ctx->sendTXT("{\"type\":\"iat_end\"}");
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}
