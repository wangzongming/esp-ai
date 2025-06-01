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
#include "lights.h"
void ESP_AI::lights_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->lights();
}

// 根据位置返回对应的颜色
uint32_t wheel(byte pos)
{
    if (pos < 85)
    {
        return esp_ai_pixels.Color(pos * 3, 255 - pos * 3, 0); // 红->绿渐变
    }
    else if (pos < 170)
    {
        pos -= 85;
        return esp_ai_pixels.Color(255 - pos * 3, 0, pos * 3); // 绿->蓝渐变
    }
    else
    {
        pos -= 170;
        return esp_ai_pixels.Color(0, pos * 3, 255 - pos * 3); // 蓝->红渐变
    }
}

void ESP_AI::lights()
{

    int rainbow_step_ai = 0;

    while (true)
    { 
        if (mp3_player_is_playing())
        {
            esp_ai_pixels.setPixelColor(0, wheel(rainbow_step_ai));
            esp_ai_pixels.show();
            rainbow_step_ai = (rainbow_step_ai + 15) & 255;
        }
        else
        {
            if (esp_ai_status == "iat_start" || esp_ai_status == "wakeup")
            {
                esp_ai_pixels.clear();
                esp_ai_pixels.setPixelColor(0, esp_ai_pixels.Color(18, 170, 156));
                esp_ai_pixels.setBrightness(100);
            }

            else if (esp_ai_status == "iat_end" || esp_ai_status == "tts_real_end")
            {
                esp_ai_pixels.clear();
            }
            else if (esp_ai_status == "0_ing")
            {
                esp_ai_pixels.setPixelColor(0, esp_ai_pixels.Color(238, 39, 70));
                esp_ai_pixels.setBrightness(50);
            }
            else if (esp_ai_status == "0_ing_after")
            {
                esp_ai_pixels.clear();
            }
            // 未初始化完毕
            else if (esp_ai_status == "0")
            {
                esp_ai_pixels.setPixelColor(0, esp_ai_pixels.Color(255, 0, 0));
                esp_ai_pixels.setBrightness(100);
            }
            else if (esp_ai_status == "0_ap")
            {
                esp_ai_pixels.setPixelColor(0, esp_ai_pixels.Color(241, 202, 23));
                esp_ai_pixels.setBrightness(100);
            }
            else if (esp_ai_status == "2")
            {
                esp_ai_pixels.setPixelColor(0, esp_ai_pixels.Color(238, 39, 70));
                esp_ai_pixels.setBrightness(50);
            }
            else if (esp_ai_status == "3")
            {
                esp_ai_pixels.clear();
            }
            else if (esp_ai_status == "long_press_ing")
            {
                esp_ai_pixels.setPixelColor(0, esp_ai_pixels.Color(238, 39, 70));
                esp_ai_pixels.setBrightness(50);
            }
            esp_ai_pixels.show();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}