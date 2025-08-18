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

#include "play_audio.h"

StaticTask_t playAudioTaskBuffer;
StackType_t playAudioTaskStack[PLAY_AUDIO_TASK_SIZE];

void play_audio_task_static(void *arg)
{
    PlayAudioContext *ctx = static_cast<PlayAudioContext *>(arg);
    if (!ctx)
    {
        Serial.println(F("[Error] PlayAudioContext ctx is null!"));
        vTaskDelete(NULL);
        return;
    }

    int frequency = 1000; // 上报间隔
    long prev_time = millis();
    long prev_up_time = 0;
    bool send0_done = true;

    char msg_buf[256]; // 用于构造 JSON 消息，避免频繁堆分配

    while (true)
    {
        int available = ctx->available();

        // 有音频数据并且正在播放
        if (available > 0 && *(ctx->spk_ing)) 
        {
            if (*(ctx->esp_ai_ws_connected) && send0_done)
            {
                send0_done = false;
            } 
            ctx->copy();
        }

        // 上报可用缓冲区信息
        if (*(ctx->esp_ai_ws_connected) &&
            (((millis() - prev_time) >= frequency) || (available < (AUDIO_BUFFER_SIZE / 2))) &&
            !send0_done && *(ctx->spk_ing))
        {
            if ((millis() - prev_up_time) > 500)
            {
                prev_time = millis();
                prev_up_time = millis();

                if (available == 0)
                {
                    send0_done = true;
                }
 
                snprintf(msg_buf, sizeof(msg_buf),
                         "{ \"type\":\"client_available_audio\", \"session_id\": \"%s\", \"value\": \"%d\" }",
                         ctx->esp_ai_session_id->c_str(),
                         available);

                ctx->sendTXT(msg_buf);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
