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
    if (esp_ai_ws_connected)
    {
        esp_ai_session_id = "";
        esp_ai_tts_task_id = "";
        esp_ai_start_get_audio = false; 
        asr_ing = true; 
        
        recive_status = false;
        while (write_status)
        { 
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        vTaskDelay(pdMS_TO_TICKS(200));
        esp_ai_dec.end();

        esp_ai_volume.setVolume(0); 
        delay(30); 
 
        esp_ai_dec.begin();   
        delay(200); 

        esp_ai_volume.setVolume(volume_config.volume); 
        delay(200); 
         
        
        // 播放问候语
        if (scene == "wakeup" && !esp_ai_cache_audio_greetings.empty() && !esp_ai_is_listen_model)
        { 
            size_t bytes_written = esp_ai_dec.write(esp_ai_cache_audio_greetings.data(), esp_ai_cache_audio_greetings.size()); 
            int num_samples = esp_ai_cache_audio_greetings.size() / 2;
            int duration_ms = (num_samples * 1000) / 16000; 
            delay(duration_ms);   
        } 
 
        
        // 播放提示音
        if (!esp_ai_cache_audio_du.empty())
        {
            esp_ai_dec.write(esp_ai_cache_audio_du.data(), esp_ai_cache_audio_du.size()); 
            int num_samples = esp_ai_cache_audio_du.size() / 2;
            int duration_ms = (num_samples * 1000) / 16000;
            delay(duration_ms);  
        }

        last_silence_time = 0;
        wakeup_time = millis();

        // 继续采集音频
        esp_ai_start_get_audio = true; 

        // 清空缓冲区
        if (esp_ai_asr_sample_buffer_before && !esp_ai_asr_sample_buffer_before->empty())
        {
            esp_ai_asr_sample_buffer_before->clear();
        }
 
        esp_ai_dec.end(); 

        // 提示音播放完后发送 start
        DEBUG_PRINTLN(debug, F("[Info] -> 发送 start"));
        esp_ai_webSocket.sendTXT("{ \"type\":\"start\" }");
        DEBUG_PRINTLN(debug, F("[Info] -> 开始录音"));
        recive_status = true;

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
