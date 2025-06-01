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
                            play_builtin_audio(san_ci, san_ci_len);
                        }
                        if (click_count == 5)
                        {
                            click_count = 0;
                            last_btn_time = 0;

                            play_builtin_audio(hui_fu_chu_chang, hui_fu_chu_chang_len);
                            wait_mp3_player_done();

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