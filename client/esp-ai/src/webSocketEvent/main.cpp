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
#include "main.h"

void ESP_AI::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        if (esp_ai_ws_connected)
        {
            esp_ai_ws_connected = false;
            esp_ai_start_ed = "0";
            esp_ai_session_id = "";
            Serial.println("\n\n[Info] -> ESP-AI 服务已断开\n\n");
            esp_ai_cache_audio_du.clear();
            esp_ai_cache_audio_greetings.clear();
            esp_ai_cache_audio_sleep_reply.clear();

            // 内置状态处理
            status_change("2");
            // 设备状态回调
            if (onNetStatusCb != nullptr)
            {
                esp_ai_net_status = "2";
                onNetStatusCb("2");
            }
        }
        break;
    case WStype_CONNECTED:
    {
        Serial.println("\n\n[Info] -> ESP-AI 服务连接成功\n\n");
        esp_ai_ws_connected = true;
        esp_ai_start_ed = "0";
        esp_ai_session_id = "";

        esp_ai_start_get_audio = false;
        esp_ai_start_send_audio = false;

        JSONVar data_1;
        data_1["type"] = "play_audio_ws_conntceed";
        String sendData = JSON.stringify(data_1);
        esp_ai_webSocket.sendTXT(sendData);
        // 内置状态处理
        status_change("3");
        // 设备状态回调
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "3";
            onNetStatusCb("3");
        }
        break;
    }
    case WStype_TEXT:
        if (strcmp((char *)payload, "session_end") == 0)
        {
            DEBUG_PRINTLN(debug, ("\n[Info] -> 会话结束\n"));

            // 没有会话ID时，说明不是会话状态，无需回复休息语
            if (!esp_ai_cache_audio_sleep_reply.empty() && esp_ai_session_id != "" && !esp_ai_is_listen_model)
            {
                esp_ai_dec.begin();
                esp_ai_dec.write(esp_ai_cache_audio_sleep_reply.data(), esp_ai_cache_audio_sleep_reply.size());
            }

            esp_ai_start_ed = "0";
            esp_ai_session_id = "";
        }
        else
        {
            JSONVar parseRes = JSON.parse((char *)payload);
            if (JSON.typeof(parseRes) == "undefined")
            {
                return;
            }
            if (parseRes.hasOwnProperty("type"))
            {
                String type = (const char *)parseRes["type"];
                String command_id = "";
                String data = "";
                if (parseRes.hasOwnProperty("command_id"))
                {
                    command_id = (const char *)parseRes["command_id"];
                }
                if (parseRes.hasOwnProperty("data"))
                {
                    data = (const char *)parseRes["data"];
                }

                if (type == "stc_time")
                {
                    String stc_time = parseRes["stc_time"];
                    JSONVar data_delayed;
                    data_delayed["type"] = "cts_time";
                    data_delayed["stc_time"] = stc_time;
                    String sendData = JSON.stringify(data_delayed);
                    esp_ai_webSocket.sendTXT(sendData);
                }

                else if (type == "net_delay")
                {
                    long net_delay = parseRes["net_delay"];
                    DEBUG_PRINTLN(debug, ("\n======================================="));
                    DEBUG_PRINTLN(debug, "网络延时：" + String(net_delay) + "ms");
                    DEBUG_PRINTLN(debug, ("=======================================\n"));
                }

                // user command
                else if (type == "instruct")
                {
                    DEBUG_PRINTLN(debug, "[instruct] -> 客户端收到用户指令：" + command_id + " --- " + data);
                    if (onEventCb != nullptr)
                    {
                        onEventCb(command_id, data);
                    }
                }

                // tts task log
                else if (type == "play_audio")
                {
                    // 用户在说话时丢弃服务数据
                    if (esp_ai_start_get_audio)
                    {
                        esp_ai_tts_task_id = "";
                        return;
                    }

                    // 结束解码
                    esp_ai_dec.end();
                    delay(100);

                    esp_ai_dec.begin();
                    esp_ai_tts_task_id = (const char *)parseRes["tts_task_id"];
                    String now_session_id = (const char *)parseRes["session_id"];
                    DEBUG_PRINTLN(debug, "\n[TTS] -> TTS 任务：" + esp_ai_tts_task_id + " 所属会话：" + now_session_id);
                }
                else if (type == "session_start")
                {
                    String now_session_id = (const char *)parseRes["session_id"];
                    DEBUG_PRINTLN(debug, "\n[Info] -> 会话开始：" + now_session_id);
                    esp_ai_session_id = now_session_id;
                }

                else if (type == "session_stop")
                {
                    // 用户在说话时丢弃服务数据
                    if (esp_ai_start_get_audio)
                    {
                        esp_ai_tts_task_id = "";
                        esp_ai_session_id = "";
                        return;
                    }

                    // 这里仅仅是停止，并不能结束录音
                    DEBUG_PRINTLN(debug, "\n[Info] -> 会话停止");
                    esp_ai_dec.end();
                    esp_ai_session_id = "";
                }

                else if (type == "auth_fail")
                {
                    String message = (const char *)parseRes["message"];
                    String code = (const char *)parseRes["code"];
                    Serial.println("[Error] -> 连接服务失败，鉴权失败：" + message);
                    Serial.println(F("[Error] -> 请检测服务器配置中是否配置了鉴权参数。"));
                    Serial.println(F("[Error] -> 如果你想用开放平台服务请到配网页面配置秘钥！"));
                    Serial.println(F("[Error] -> 如果你想用开放平台服务请到配网页面配置秘钥！"));
                    Serial.println(F("[Error] -> 如果你想用开放平台服务请到配网页面配置秘钥！"));
                    if (onErrorCb != nullptr)
                    {
                        onErrorCb("002", "auth", message);
                    }
                }
                else if (type == "error")
                {
                    String at_pos = (const char *)parseRes["at"];
                    String message = (const char *)parseRes["message"];
                    String code = (const char *)parseRes["code"];
                    Serial.println("[Error] -> 服务错误：" + at_pos + " " + code + " " + message);

                    if (code == "4002")
                    {
                        esp_ai_dec.end();
                        delay(100);
                        esp_ai_dec.begin();
                        esp_ai_dec.write(yu_e_bu_zuo, yu_e_bu_zuo_len);
                    }
                    else if (code == "4001")
                    {
                        esp_ai_dec.end();
                        delay(100);
                        esp_ai_dec.begin();
                        esp_ai_dec.write(e_du_ka_bu_cun_zai, e_du_ka_bu_cun_zai_len);
                    }
                    else if (code == "4000")
                    {
                        esp_ai_dec.end();
                        delay(100);
                        esp_ai_dec.begin();
                        esp_ai_dec.write(chao_ti_wei_qi_yong, chao_ti_wei_qi_yong_len);
                    }

                    if (onErrorCb != nullptr)
                    {
                        onErrorCb(code, at_pos, message);
                    }
                }

                else if (type == "session_status")
                {
                    String status = (const char *)parseRes["status"];
                    DEBUG_PRINTLN(debug, "[Info] -> 会话状态：" + status);

                    if (status == "iat_end")
                    {
                        esp_ai_start_ed = "0";
                        esp_ai_start_get_audio = false;
                        esp_ai_start_send_audio = false;
                    }
                    else if (status == "iat_start")
                    {

                        // 正在说话时就不要继续推理了，否则会误唤醒
                        esp_ai_start_ed = "1";
                        // 开始发送音频时，将缓冲区中的数据发送出去
                        esp_ai_start_send_audio = true;
                    }

                    // 内置状态处理
                    status_change(status);
                    if (onSessionStatusCb != nullptr)
                    {
                        onSessionStatusCb(status);
                    }
                }
                else if (type == "set_wifi_config")
                {

                    JSONVar JSON_data = parseRes["configs"];
                    bool is_ok = setWifiConfig(JSON_data);

                    JSONVar set_wifi_config_res;
                    set_wifi_config_res["type"] = "set_wifi_config_res";
                    set_wifi_config_res["success"] = is_ok;
                    String sendData = JSON.stringify(data);
                    DEBUG_PRINTLN(debug, F("\n[TTS] -> 发送设置WiFi参数结果到服务端"));
                    esp_ai_webSocket.sendTXT(sendData);
                }

                else if (type == "restart")
                {
                    ESP.restart();
                }

                else if (type == "set_local_data")
                {
                    String field = (const char *)parseRes["field"];
                    String value = (const char *)parseRes["value"];
                    set_local_data(field, value);
                }
                else if (type == "log")
                {
                    String data = (const char *)parseRes["data"];
                    DEBUG_PRINT(debug, F("\n[Server Log] -> "));
                    DEBUG_PRINTLN(debug, data);
                }
                else if (type == "sever-close")
                {
                    DEBUG_PRINT(debug, F("\n[Error] 服务端主动断开，尝试重新连接。"));
                    ESP.restart();
                }

                else if (type == "hardware-fns")
                {
                    int pin = (int)parseRes["pin"];
                    String fn_name = (const char *)parseRes["fn_name"];
                    String str_val = (const char *)parseRes["str_val"];
                    int num_val = (int)parseRes["num_val"];

                    if (fn_name == "pinMode")
                    {
                        str_val == "OUTPUT" && (pinMode(pin, OUTPUT), true);
                        str_val == "INPUT" && (pinMode(pin, INPUT), true);
                        str_val == "INPUT_PULLUP" && (pinMode(pin, INPUT_PULLUP), true);
                        str_val == "INPUT_PULLDOWN" && (pinMode(pin, INPUT_PULLDOWN), true);
                    }
                    else if (fn_name == "digitalWrite")
                    {
                        str_val == "HIGH" && (digitalWrite(pin, HIGH), true);
                        str_val == "LOW" && (digitalWrite(pin, LOW), true);
                    }
                    else if (fn_name == "digitalRead")
                    {
                        digital_read_pins.push_back(pin);
                    }
                    else if (fn_name == "analogWrite")
                    {
                        analogWrite(pin, num_val);
                    }
                    else if (fn_name == "analogRead")
                    {
                        analog_read_pins.push_back(pin);
                    }
                }
            }
        }

        if (debug)
        {
            Serial.printf("[Info] -> Received Text: %s\n", payload);
        }
        break;
    case WStype_BIN:
    {
        char session_id_string[5];
        memcpy(session_id_string, payload, 4);
        session_id_string[4] = '\0';
        String sid = String(session_id_string);

        /**
         * sid
         * 0000 -> 嘟提示音数据
         * 0001 -> 服务连接成功提示语
         * 1000 -> 提示音缓存数据
         * 1001 -> 唤醒问候语缓存数据
         * 1002 -> 休息时回复缓存数据
         * 2000 -> 整个回复的TTS最后一组数据，需要继续对话
         * 2001 -> 整个回复的TTS最后一组数据，无需继续对话
         * 2002 -> TTS 任务组的片段完毕
         * 其他 -> session_id
         */

        if (sid == "2000")
        {
            esp_ai_tts_task_id = "";
            // 内置状态处理
            status_change("tts_real_end");

            if (esp_ai_session_id != "")
            {
                if (onSessionStatusCb != nullptr)
                {
                    onSessionStatusCb("tts_real_end");
                }

                if (!esp_ai_is_listen_model)
                {
                    // tts发送完毕，需要重新开启录音
                    DEBUG_PRINTLN(debug, ("\n[TTS] -> TTS 数据全部接收完毕，需继续对话。"));
                    wakeUp("continue");
                }
            }
        }
        else if (sid == "2001")
        {  
            if (esp_ai_played_connected == false && esp_ai_prev_session_id == "0001")
            {
                esp_ai_played_connected = true;
            }

            esp_ai_tts_task_id = "";
            // 内置状态处理
            status_change("tts_real_end");

            if (esp_ai_session_id != "")
            {
                if (onSessionStatusCb != nullptr)
                {
                    onSessionStatusCb("tts_real_end");
                }
                DEBUG_PRINTLN(debug, ("\n[TTS] -> TTS 数据全部接收完毕，无需继续对话。"));
            }
            esp_ai_prev_session_id = sid;
            return;
        }
        else if (sid == "2002")
        {
            DEBUG_PRINTLN(debug, ("\n[TTS] -> TTS CHUNK 接收完毕。"));
            esp_ai_tts_task_id = "";
        } 
        
        if (sid == "0001" && esp_ai_played_connected == true)
        {
            esp_ai_prev_session_id = sid;
            return;
        }
        // 提取音频数据
        uint8_t *audioData = payload + 4;
        size_t audioLength = length - 4;

        if (sid == "1000")
        {
            esp_ai_cache_audio_du.insert(esp_ai_cache_audio_du.end(), audioData, audioData + audioLength);
        }
        else if (sid == "1001")
        {
            esp_ai_cache_audio_greetings.insert(esp_ai_cache_audio_greetings.end(), audioData, audioData + audioLength);
        }
        else if (sid == "1002")
        {
            esp_ai_cache_audio_sleep_reply.insert(esp_ai_cache_audio_sleep_reply.end(), audioData, audioData + audioLength);
        }

        if (session_id_string && sid != "0000" && sid != "0001" && sid != esp_ai_session_id)
        {
            esp_ai_prev_session_id = sid;
            return;
        }
        esp_ai_dec.write(audioData, audioLength);
        esp_ai_prev_session_id = sid;
        break;
    }
    // case WStype_PING:
    //     Serial.println("Ping");
    //     break;
    // case WStype_PONG:
    //     Serial.println("Pong");
    //     break;
    case WStype_ERROR:
        Serial.println("[Error] 服务 WebSocket 连接错误");
        break;
    }
}
