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
#include "send_audio.h"

void ESP_AI::send_audio_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->send_audio();
}

void ESP_AI::send_audio()
{
    int threshold = 1028 * 3; 
    long last_send_time = 0;

    while (true)
    {
        if (esp_ai_start_send_audio)
        { 
            int cur_size = esp_ai_asr_sample_buffer_before->size();   

            if (cur_size >= threshold)
            {
                std::vector<uint8_t> send_buffer;

                if (esp_ai_is_first_send)
                {
                    uint8_t mp3_header[4] = {0xFF, 0xFB, 0x90, 0x64}; // MP3 标准帧头：单声道、16kHz、128kbps
                    send_buffer.insert(send_buffer.end(), mp3_header, mp3_header + sizeof(mp3_header));
                    esp_ai_is_first_send = false;
                }

                // 添加实际 MP3 数据
                send_buffer.insert(
                    send_buffer.end(),
                    esp_ai_asr_sample_buffer_before->begin(),
                    esp_ai_asr_sample_buffer_before->begin() + threshold);

                esp_ai_webSocket.sendBIN(send_buffer.data(), send_buffer.size());
                last_send_time = millis();

                // 清除已经发送的部分
                esp_ai_asr_sample_buffer_before->erase(
                    esp_ai_asr_sample_buffer_before->begin(),
                    esp_ai_asr_sample_buffer_before->begin() + threshold);
            }
            else if (esp_ai_is_listen_model && !esp_ai_start_get_audio)
            {
                if (cur_size == 0)
                {
                    DEBUG_PRINTLN(debug, ("大语言模型正在推理。"));
                    esp_ai_start_get_audio = false;
                    esp_ai_start_send_audio = false;
                    esp_ai_webSocket.sendTXT("{\"type\":\"iat_end\"}");
                }
                else
                {
                    // 这里只要数据不是空的都应该全部发送给服务器 
                    if (!esp_ai_asr_sample_buffer_before->empty()) 
                    {
                        esp_ai_webSocket.sendBIN(esp_ai_asr_sample_buffer_before->data(), esp_ai_asr_sample_buffer_before->size());
                        esp_ai_asr_sample_buffer_before->clear();
                    }
                }
            }

            vTaskDelay(40 / portTICK_PERIOD_MS);
        }
        else
        {
            if (!esp_ai_asr_sample_buffer_before->empty())
            {
                esp_ai_asr_sample_buffer_before->clear();
            }
            esp_ai_is_first_send = true; 
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    vTaskDelete(NULL);
}