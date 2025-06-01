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
            asr_ing = false;
            Serial.print("[Info] -> ESP-AI 服务已断开：");
            Serial.println(length);
            esp_ai_cache_audio_du.clear();
            esp_ai_cache_audio_greetings.clear();
            // esp_ai_cache_audio_sleep_reply.clear();

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
        Serial.println("[Info] -> ESP-AI 服务连接成功");
        esp_ai_ws_connected = true;
        esp_ai_start_ed = "0";
        esp_ai_session_id = "";
        asr_ing = false;
        spk_ing = false;

        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            JSONVar data_1;
            data_1["type"] = "play_audio_ws_conntceed";
            String sendData = JSON.stringify(data_1);
            esp_ai_webSocket.sendTXT(sendData);
            xSemaphoreGive(esp_ai_ws_mutex);
        }

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
            esp_ai_start_ed = "0";
            esp_ai_session_id = "";
            esp_ai_tts_task_id = "";
            esp_ai_status = "3";
            asr_ing = false;
            spk_ing = false;
        }
        else
        {
            JSONVar parseRes = JSON.parse((char *)payload);
            if (JSON.typeof(parseRes) == "undefined")
            {
                return;
            }
            if (debug)
            {
                Serial.print(F("[Info] -> Received Text: "));
                Serial.println((char *)payload);
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
                    if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                    {
                        String stc_time = parseRes["stc_time"];
                        JSONVar data_delayed;
                        data_delayed["type"] = "cts_time";
                        data_delayed["stc_time"] = stc_time;
                        String sendData = JSON.stringify(data_delayed);
                        esp_ai_webSocket.sendTXT(sendData);
                        xSemaphoreGive(esp_ai_ws_mutex);
                    }
                }

                else if (type == "net_delay")
                {
                    long net_delay = parseRes["net_delay"];
                    DEBUG_PRINTLN(debug, "[Info] -> 网络延时：" + String(net_delay) + "ms");
                }

                // user command
                else if (type == "instruct")
                {
                    if (onEventCb != nullptr)
                    {
                        onEventCb(command_id, data);
                    }
                }

                // tts task log
                else if (type == "play_audio")
                {
                    // 上报音频时
                    if (esp_ai_start_send_audio)
                    {
                        esp_ai_tts_task_id = "";
                        return;
                    }
                    spk_ing = true;
                    esp_ai_tts_task_id = (const char *)parseRes["tts_task_id"];
                    String now_session_id = (const char *)parseRes["session_id"];
                    DEBUG_PRINTLN(debug, "[TTS] -> TTS 任务：" + esp_ai_tts_task_id + " 所属会话：" + now_session_id);
                }
                else if (type == "session_start")
                {
                    esp_ai_session_id = (const char *)parseRes["session_id"];
                    spk_ing = true;
                }

                else if (type == "session_stop")
                {
                    // 上报音频时
                    if (esp_ai_start_send_audio)
                    {
                        esp_ai_tts_task_id = "";
                        return;
                    }

                    // 这里仅仅是停止，并不能结束录音
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
                        play_builtin_audio(yu_e_bu_zuo, yu_e_bu_zuo_len);
                    }
                    else if (code == "4001")
                    {
                        play_builtin_audio(e_du_ka_bu_cun_zai, e_du_ka_bu_cun_zai_len);
                    }
                    else if (code == "4000")
                    {
                        play_builtin_audio(chao_ti_wei_qi_yong, chao_ti_wei_qi_yong_len);
                    }

                    if (onErrorCb != nullptr)
                    {
                        onErrorCb(code, at_pos, message);
                    }
                }

                else if (type == "session_status")
                {
                    String status = (const char *)parseRes["status"];

                    if (status == "iat_end")
                    {
                        esp_ai_start_ed = "0";
                        esp_ai_start_send_audio = false;
                        asr_ing = false;
                        spk_ing = true;
                    }
                    else if (status == "iat_start")
                    {
                        // 开始发送音频时，先等话说完
                        wait_mp3_player_done();
                        // 正在说话时就不要继续推理了，否则会误唤醒
                        esp_ai_start_ed = "1";
                        // 开始发送音频时，将缓冲区中的数据发送出去
                        esp_ai_start_send_audio = true;
                        // 记录时间戳
                        last_silence_time = millis();
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

                    if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                    {

                        JSONVar JSON_data = parseRes["configs"];
                        bool is_ok = setWifiConfig(JSON_data);

                        JSONVar set_wifi_config_res;
                        set_wifi_config_res["type"] = "set_wifi_config_res";
                        set_wifi_config_res["success"] = is_ok;
                        String sendData = JSON.stringify(data);
                        DEBUG_PRINTLN(debug, F("[TTS] -> 发送设置WiFi参数结果到服务端"));
                        esp_ai_webSocket.sendTXT(sendData);
                        xSemaphoreGive(esp_ai_ws_mutex);
                    }
                }

                else if (type == "restart")
                {
                    ESP.restart();
                }
                else if (type == "clear_cache")
                {
                    if (!esp_ai_cache_audio_du.empty())
                    {
                        esp_ai_cache_audio_du.clear();
                    }
                    if (!esp_ai_cache_audio_greetings.empty())
                    {
                        esp_ai_cache_audio_greetings.clear();
                    }
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
                }
                else if (type == "sever-close")
                {
                    DEBUG_PRINT(debug, F("[Error] 服务端主动断开，尝试重新连接。"));
                    ESP.restart();
                }

                else if (type == "hardware-fns")
                {
                    int pin = (int)parseRes["pin"];
                    String fn_name = (const char *)parseRes["fn_name"];
                    String str_val = (const char *)parseRes["str_val"];
                    int num_val = (int)parseRes["num_val"];

                    // 设置引脚模式
                    if (fn_name == "pinMode")
                    {
                        str_val == "OUTPUT" && (pinMode(pin, OUTPUT), true);
                        str_val == "INPUT" && (pinMode(pin, INPUT), true);
                        str_val == "INPUT_PULLUP" && (pinMode(pin, INPUT_PULLUP), true);
                        str_val == "INPUT_PULLDOWN" && (pinMode(pin, INPUT_PULLDOWN), true);

                        // LEDC
                        if (str_val == "LEDC")
                        {
                            // LEDC 通道, 取值 0 ~ 15
                            int channel = 0;
                            if (parseRes.hasOwnProperty("channel"))
                            {
                                channel = (int)parseRes["channel"];
                            }
                            // 定义 PWM 频率，舵机通常使用 50Hz
                            int freq = 50;
                            if (parseRes.hasOwnProperty("freq"))
                            {
                                freq = (int)parseRes["freq"];
                            }
                            // 定义 PWM 分辨率
                            int resolution = 10;
                            if (parseRes.hasOwnProperty("resolution"))
                            {
                                resolution = (int)parseRes["resolution"];
                            }

                            // 初始化 LEDC 通道
                            ledcSetup(channel, freq, resolution);
                            // 将 LEDC 通道绑定到指定引脚
                            ledcAttachPin(pin, channel);
                        }
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
                    // 舵机驱动
                    else if (fn_name == "ledcWrite")
                    {
                        int channel = 0;
                        if (parseRes.hasOwnProperty("channel"))
                        {
                            channel = (int)parseRes["channel"];
                        }
                        int deg = (int)parseRes["deg"];
                        ledcWrite(channel, angleToDutyCycle(deg));
                    }
                }
                // 情绪监听
                else if (type == "emotion")
                {
                    if (onEmotionCb != nullptr)
                    {
                        onEmotionCb(data);
                    }
                }
            }
        }

        break;
    case WStype_BIN:
    {

        if (length < 6)
        {
            Serial.print("[Error] -> 数据帧长度小于6字节: ");
            Serial.println(length);
            return;
        }

        // 会话ID
        char session_id_string[5];
        memcpy(session_id_string, payload, 4);
        session_id_string[4] = '\0';
        String sid = String(session_id_string);

        // 会话状态
        char session_status_string[3];
        memcpy(session_status_string, payload + 4, 2);
        session_status_string[2] = '\0';
        esp_ai_session_status = String(session_status_string);

        // Serial.print("内容长度：");
        // Serial.print(length);
        // Serial.print("  会话ID：");
        // Serial.print(sid);
        // Serial.print("  会话状态：");
        // Serial.print(esp_ai_session_status);

        // 提取音频数据
        uint8_t *audioData = payload + 6;
        size_t audioLength = length - 6;

        if (sid == SID_TONE_CACHE)
        {
            esp_ai_cache_audio_du.insert(esp_ai_cache_audio_du.end(), audioData, audioData + audioLength);
        }
        else if (sid == SID_WAKEUP_REP_CACHE)
        {
            esp_ai_cache_audio_greetings.insert(esp_ai_cache_audio_greetings.end(), audioData, audioData + audioLength);
        }
        else
        {
            // 会话ID 不正确的分组数据直接抛弃  SID_WAKEUP_REP_CACHE
            if (session_id_string && sid != SID_TONE && sid != SID_CONNECTED_SERVER && sid != SID_TTS_FN && sid != esp_ai_session_id)
                return;
            if (sid == SID_CONNECTED_SERVER && esp_ai_played_connected)
                return;

            // Serial.print("  写入长度：");
            // Serial.print(audioLength);
            // Serial.print("    正在播放长度：");
            // Serial.println(esp_ai_spk_queue.available());
            mp3_player_write(audioData, audioLength);
        }

        if (esp_ai_session_status == SID_TTS_END_RESTART)
        {
            esp_ai_tts_task_id = "";
            // 内置状态处理
            status_change("tts_real_end");
            if (esp_ai_session_id != "")
            {
                wait_mp3_player_done();

                if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    esp_ai_webSocket.sendTXT("{ \"type\":\"client_out_audio_over\", \"session_id\": \"" + sid + "\",  \"session_status\": \"" + esp_ai_session_status + "\", \"tts_task_id\": \"" + esp_ai_tts_task_id + "\" }");
                    xSemaphoreGive(esp_ai_ws_mutex);
                }

                if (onSessionStatusCb != nullptr)
                {
                    onSessionStatusCb("tts_real_end");
                }

                if (!esp_ai_is_listen_model)
                {
                    // tts发送完毕，需要重新开启录音
                    DEBUG_PRINTLN(debug, F("[TTS] -> TTS 数据全部接收完毕，需继续对话。"));
                    // 等待音频播放完毕
                    // wait_mp3_player_done();
                    asr_ing = false;
                    spk_ing = false;
                    wakeUp("continue");
                }
            }
        }
        else if (esp_ai_session_status == SID_TTS_END)
        {

            // 服务连接成功播放完毕
            bool is_first_connect = esp_ai_played_connected == false && sid == SID_CONNECTED_SERVER && esp_ai_session_status == SID_TTS_END;
            if (is_first_connect)
            {
                esp_ai_played_connected = true;
            }

            wait_mp3_player_done();
            if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                esp_ai_webSocket.sendTXT("{ \"type\":\"client_out_audio_over\", \"session_id\": \"" + sid + "\", \"session_status\": \"" + esp_ai_session_status + "\", \"tts_task_id\": \"" + esp_ai_tts_task_id + "\"}");
                xSemaphoreGive(esp_ai_ws_mutex);
            }

            esp_ai_tts_task_id = "";
            // 内置状态处理
            status_change("tts_real_end");

            if (onSessionStatusCb != nullptr)
            {
                onSessionStatusCb("tts_real_end");
            }
            DEBUG_PRINT(debug, F("[TTS] -> TTS 数据全部接收完毕，无需继续对话"));
            esp_ai_start_ed = "0";

            // 服务连接成功播放完毕
            if (is_first_connect && onReadyCb != nullptr)
            {
                wait_mp3_player_done();
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                onReadyCb();
            }
            return;
        }
        else if (esp_ai_session_status == SID_TTS_CHUNK_END)
        {
            esp_ai_tts_task_id = "";
        }
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
