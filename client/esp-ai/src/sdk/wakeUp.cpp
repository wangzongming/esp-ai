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
#include "wakeUp.h"

void ESP_AI::wakeUp(String scene)
{
    if (esp_ai_ws_connected)
    {
        esp_ai_session_id = "";
        esp_ai_tts_task_id = "";
        esp_ai_start_get_audio = false;

        // 结束解码
        esp_ai_dec.end();
        delay(100);
 
        esp_ai_dec.begin();
        // 播放问候语
        if (scene == "wakeup" && !esp_ai_cache_audio_greetings.empty() && !esp_ai_is_listen_model)
        {
            esp_ai_dec.write(esp_ai_cache_audio_greetings.data(), esp_ai_cache_audio_greetings.size());
            delay(300);
        }

        // 播放提示音
        if (!esp_ai_cache_audio_du.empty())
        {
            esp_ai_dec.write(esp_ai_cache_audio_du.data(), esp_ai_cache_audio_du.size());
            delay(300);
        } 

        last_silence_time = 0;
        wakeup_time = millis();

        // 继续采集音频
        esp_ai_start_get_audio = true;
        esp_ai_is_first_send = true;

        // 清空缓冲区
        if (esp_ai_asr_sample_buffer_before && !esp_ai_asr_sample_buffer_before->empty())
        {
            esp_ai_asr_sample_buffer_before->clear();
        }
        else
        {
            DEBUG_PRINTLN(debug, ("[ERROR] -> 尝试清理空的esp_ai_asr_sample_buffer_before"));
        }
 
        // 提示音播放完后发送 start
        DEBUG_PRINTLN(debug, ("[Info] -> 发送 start"));
        esp_ai_webSocket.sendTXT("{ \"type\":\"start\" }");
        DEBUG_PRINTLN(debug, ("[Info] -> 开始录音"));

        // 内置状态处理
        if (scene == "wakeup")
        {
            status_change("wakeup");
            if (onSessionStatusCb != nullptr)
            {
                onSessionStatusCb("wakeup");
            }
        }

        esp_ai_user_has_spoken = false;
    }
    else
    {
        Serial.println(F("[Error] -> 请先连接服务器"));
    }
}
