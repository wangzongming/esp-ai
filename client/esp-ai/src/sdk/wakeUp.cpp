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

void ESP_AI::wakeUp()
{
    if (ws_connected)
    {
        // 音频流清空 
        esp_ai_dec.end();
        delay(300);
        esp_ai_dec.begin();

        // 开始录音
        // digitalWrite(LED_BUILTIN, HIGH);
        start_ed = "1";
        session_id = "";

        JSONVar data;
        data["type"] = "start";
        String sendData = JSON.stringify(data);
        esp_ai_webSocket.sendTXT(sendData);
        DEBUG_PRINTLN(debug, ("[Info] -> 开始录音"));
    }
    else
    {
        Serial.println(F("[Error] -> 请先连接服务器"));
    }
}
