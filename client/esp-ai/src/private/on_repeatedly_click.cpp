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
#include "on_repeatedly_click.h"

void ESP_AI::on_repeatedly_click_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->on_repeatedly_click();
}

void ESP_AI::on_repeatedly_click()
{
    long esp_ai_last_debounce_time = 0;
    int esp_ai_debounce_delay = 250;
    int esp_ai_prev_state = reset_btn_config.power == "high" ? 0 : 1;
    int esp_ai_prev_state_listen = 0;

    long last_btn_time = 0;
    int click_count = 0;
    int click_max_delay = 1000; 

    while (true)
    {
        int reading = digitalRead(reset_btn_config.pin);
        long curTime = millis();
        int target_val = reset_btn_config.power == "high" ? 1 : 0;
        if (reading == target_val)
        { 
            if ((curTime - esp_ai_last_debounce_time) > esp_ai_debounce_delay)
            {
                esp_ai_last_debounce_time = curTime;
                if (esp_ai_prev_state != reading)
                {
                    esp_ai_prev_state = reading;

                    if ((last_btn_time == 0) || (millis() - last_btn_time) <= click_max_delay)
                    {
                        last_btn_time = millis();
                        click_count++; 
                        if (click_count == 2)
                        {
                            play_cache = "clear_cache";
                        }
                        if (click_count == 5)
                        {
                            click_count = 0;
                            last_btn_time = 0;

                            // 结束对话音频
                            esp_ai_session_id = "";
                            DEBUG_PRINTLN(debug, F("触发重置设置"));
                            if (onRepeatedlyClickCb != nullptr)
                            {
                                onRepeatedlyClickCb();
                            }
                            clearData();
                            ESP.restart();
                            // 删除任务
                            vTaskDelete(NULL);
                        }
                    }
                }
            }
        }
        else
        {
            esp_ai_prev_state = reading;
            if (click_count != 0 && ((millis() - last_btn_time) >= click_max_delay))
            {
                click_count = 0;
                last_btn_time = 0;
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}