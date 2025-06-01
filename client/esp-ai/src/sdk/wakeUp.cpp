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
#include "wakeUp.h"

void ESP_AI::wakeUp(String scene)
{
    if (asr_ing)
    {
        DEBUG_PRINTLN(debug, F("[Info] -> 正在聆听中，唤醒无效。"));
        return;
    }

    if (esp_ai_ws_connected)
    {
        esp_ai_session_id = ""; 
        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // 提示服务器start
            DEBUG_PRINTLN(debug, F("[Info] -> 发送 start"));
            esp_ai_webSocket.sendTXT("{ \"type\":\"start\" }");

            // esp_ai_webSocket.disableHeartbeat();
            xSemaphoreGive(esp_ai_ws_mutex);
        }
 

        mp3_player_stop();

        // 播放问候语
        if (scene == "wakeup" && !esp_ai_cache_audio_greetings.empty() && !esp_ai_is_listen_model)
        {
            play_builtin_audio(esp_ai_cache_audio_greetings.data(), esp_ai_cache_audio_greetings.size());
            wait_mp3_player_done();
        }

        // 播放提示音
        if (!esp_ai_cache_audio_du.empty())
        {
            play_builtin_audio(esp_ai_cache_audio_du.data(), esp_ai_cache_audio_du.size());
            wait_mp3_player_done();
        }

        // 清空缓冲区
        memset(esp_ai_asr_sample_buffer, 0, sizeof(esp_ai_asr_sample_buffer));

        // 内置状态处理
        if (scene == "wakeup")
        {
            status_change("wakeup");
            if (onSessionStatusCb != nullptr)
            {
                onSessionStatusCb("wakeup");
            }
        }

        esp_ai_tts_task_id = "";
        asr_ing = true;
        spk_ing = false;
    }
    else
    {
        Serial.println(F("[Error] -> 请先连接服务器"));
    }
}
