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
void ESP_AI::play_audio_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->play_audio();
}

void ESP_AI::play_audio()
{
    // 发送正在可用音频流的频率
    int frequency = 1000;
    long prev_time = millis();
    // 是否已经发送过可用为 0 的数据
    bool send0_ed = false;

    while (true)
    {
        int available = esp_ai_spk_queue.available();
        if (available > 0)
        {
            if (send0_ed)
            {
                send0_ed = false;
            }
            esp_ai_copier.copy();
        }
        if (esp_ai_ws_connected && ((millis() - prev_time) > frequency) && !send0_ed)
        {
            prev_time = millis();
            if (available == 0)
            {
                send0_ed = true;
            }
            if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                esp_ai_webSocket.sendTXT("{ \"type\":\"client_available_audio\", \"session_id\": \"" + esp_ai_session_id + "\", \"value\": \"" + available + "\"}");

                xSemaphoreGive(esp_ai_ws_mutex);
            }
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}
