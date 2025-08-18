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

StaticTask_t lightsTaskBuffer;
StackType_t lightsTaskStack[LIGHTS_TASK_SIZE];

// 根据位置返回对应的颜色
uint32_t wheel(Adafruit_NeoPixel *esp_ai_pixels, byte pos)
{
    int c0 = 0, c1 = 0, c2 = 0;
    if (pos < 85)
    {
        c0 = pos * 3;       // 绿色分量
        c1 = 255 - pos * 3; // 红色分量
        c2 = 0;             // 蓝色分量
    }
    else if (pos < 170)
    {
        pos -= 85;
        c0 = 255 - pos * 3; // 红色分量
        c1 = 0;             // 绿色分量
        c2 = pos * 3;       // 蓝色分量
    }
    else
    {
        pos -= 170;
        c0 = 0;             // 红色分量
        c1 = pos * 3;       // 绿色分量
        c2 = 255 - pos * 3; // 蓝色分量
    }

    return esp_ai_pixels->Color(c0, c1, c2);
}

void set_color_all(Adafruit_NeoPixel *esp_ai_pixels, int count, uint8_t brightness, uint32_t color)
{
    for (int i = 0; i < count; i++)
    {
        esp_ai_pixels->setPixelColor(i, color);
    }
    esp_ai_pixels->setBrightness(brightness);
    esp_ai_pixels->show();
}

void light_task_static(void *arg)
{

    LightContext *ctx = static_cast<LightContext *>(arg);
    if (ctx == nullptr) {
        Serial.println(F("[Error] LightContext ctx is null!"));
        vTaskDelete(NULL);
        return;
    }

    int rainbow_step_ai = 0;

    while (true)
    { 
        if (ctx->isPlaying())
        {
            set_color_all(ctx->pixels, ctx->count, 100, wheel(ctx->pixels, rainbow_step_ai));
            rainbow_step_ai = (rainbow_step_ai + 15) & 255;
        }
        else
        { 
            // if (*ctx->status == "iat_start" || *ctx->status == "wakeup")
            if (*ctx->esp_ai_start_send_audio)
            {
                ctx->pixels->clear();
                set_color_all(ctx->pixels, ctx->count, 100, ctx->pixels->Color(18, 170, 156));
            }
            else if (*ctx->status == "iat_end" || *ctx->status == "tts_real_end")
            {
                ctx->pixels->clear();
            }
            else if (*ctx->status == "0_ing")
            {
                set_color_all(ctx->pixels, ctx->count, 50, ctx->pixels->Color(238, 39, 70));
            }
            else if (*ctx->status == "0_ing_after")
            {
                ctx->pixels->clear();
            }
            // 未初始化完毕
            else if (*ctx->status == "0")
            {
                set_color_all(ctx->pixels, ctx->count, 100, ctx->pixels->Color(255, 0, 0));
            }
            else if (*ctx->status == "0_ap")
            {
                set_color_all(ctx->pixels, ctx->count, 100, ctx->pixels->Color(241, 202, 23));
            }
            else if (*ctx->status == "2")
            {
                set_color_all(ctx->pixels, ctx->count, 50, ctx->pixels->Color(238, 39, 70));
            }
            else if (*ctx->status == "3")
            {
                ctx->pixels->clear();
            }
            else if (*ctx->status == "long_press_ing")
            { 
                set_color_all(ctx->pixels, ctx->count, 50, ctx->pixels->Color(238, 39, 70));
            }
            ctx->pixels->show();
        }
 
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}