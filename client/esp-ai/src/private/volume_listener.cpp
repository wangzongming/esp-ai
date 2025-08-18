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

StaticTask_t volListenTaskBuffer;
StackType_t volListenTaskStack[VOL_LISTEN_TASK_SIZE];

void vol_listen_task_static(void *arg)
{
    VolListenContext *ctx = static_cast<VolListenContext *>(arg);
    if (!ctx)
    {
        Serial.println(F("[Error] VolListenContext ctx is null!"));
        vTaskDelete(NULL);
        return;
    }
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
        // 旋钮方式变化音量
        total -= readings[readIndex];
        readings[readIndex] = analogRead(*(ctx->pin));
        total += readings[readIndex];
        readIndex = (readIndex + 1) % numReadings;
        average = total / numReadings;
        float _t = static_cast<float>(average) / *(ctx->max_val);
        String formattedValue = String(_t, 1);
        // 音量
        float formattedNumber = formattedValue.toFloat();
        ctx->onChange(formattedNumber);

        /**
         * boot 按钮方式控制音量
         * 如果音量不是 1, 则长两秒后开始进行音量增加，增加到1后停止增加
         * 反之如果音量是 1, 则长两秒后开始进行音量减少，减少到0后停止减少
         *
         * 和对话冲突了...
         */
        // ing...

        // vTaskDelay(100 / portTICK_PERIOD_MS);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}