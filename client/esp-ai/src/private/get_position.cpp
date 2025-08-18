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
#include "get_position.h"
 
StaticTask_t getPositionContextTaskBuffer;
StackType_t getPositionContextTaskStack[GET_POSITION_TASK_SIZE];

void get_position_task_static(void *arg)
{
    GetPositionContext *ctx = static_cast<GetPositionContext *>(arg);
    if (ctx->IS_WL_CONNECTED() == false)
    {
        vTaskDelete(NULL);
        return;
    }

    String ip, nation, province, city, latitude, longitude;
    ip.reserve(32);
    nation.reserve(32);
    province.reserve(32);
    city.reserve(32);
    latitude.reserve(16);
    longitude.reserve(16);

    DEBUG_PRINTLN(ctx->debug, ("[Info] -> 正在获取位置..."));

    HTTPClient http;
    http.begin("http://api.espai.fun/sdk/position");
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        JSONVar res = JSON.parse(payload);

        if (res.hasOwnProperty("success"))
        {
            ip = (const char *)res["data"]["ip"];
            nation = (const char *)res["data"]["country_name"];
            province = (const char *)res["data"]["region"];
            city = (const char *)res["data"]["city"];
            latitude = (const char *)res["data"]["latitude"];
            longitude = (const char *)res["data"]["longitude"];

            DEBUG_PRINT(ctx->debug, "\n[Info] -> 位置获取成功: ");
            DEBUG_PRINT(ctx->debug, nation);
            DEBUG_PRINT(ctx->debug, "/");
            DEBUG_PRINT(ctx->debug, province);
            DEBUG_PRINT(ctx->debug, "/");
            DEBUG_PRINTLN(ctx->debug, city);

            if (ctx->onPositionCb)
            {
                ctx->onPositionCb(ip, nation, province, city, latitude, longitude);
            }
        }
        else
        {
            DEBUG_PRINTLN(ctx->debug, "[Warn] -> API响应格式错误，无 success 字段");
        }
    }
    else
    {
        DEBUG_PRINTLN(ctx->debug, "[Error] -> HTTP 请求失败，状态码: " + httpCode);
    }

    http.end();
    http.~HTTPClient();
    vTaskDelete(NULL);
}
