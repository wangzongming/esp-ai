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
 * 源码已经是免费使用了，如果您连版权都不保留您认为这属于什么行为？
 *
 * @author 小明IO
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */
#include "loop.h"

// int16_t diy_wakeup_sample_buffer[512];
int16_t diy_wakeup_sample_buffer[2048];

int btnClick = 0;
long lastDebounceTime = 0;
long debounceDelay = 250;
long prev_state = 0;

String cleanString(String input)
{
    String output = "";
    for (int i = 0; i < input.length(); i++)
    {
        char c = input.charAt(i);
        if (c != '\n' && c != '\r')
        {                // 如果字符不是换行符或回车符
            output += c; // 将其添加到输出字符串
        }
    }
    return output;
}

// 延迟多久发网服务器
int read_to_s_delay_time = 100;
long prev_read_to_s_delay_time = millis();

JSONVar digitalReadJSONData;
JSONVar analogReadJSONData;

void ESP_AI::loop()
{
    esp_ai_server.handleClient();
    esp_ai_webSocket.loop();

    if (WiFi.status() != WL_CONNECTED)
    {

        if (net_status == "2" && net_status != "0" && ap_connect_err != "1")
        {
            // 内置状态处理
            status_change("0");
        }
        // 设备状态回调
        if (onNetStatusCb != nullptr && net_status == "2" && net_status != "0" && ap_connect_err != "1")
        {
            net_status = "0";
            onNetStatusCb("0");
            DEBUG_PRINTLN(debug, ("WIFI 异常断开，将自动重启板子"));
            ESP.restart();
            delay(3000);
        }
    }

    if (volume_config.enable)
    {
        int _cur_ctrl_val = analogRead(volume_config.input_pin);
        float _t = static_cast<float>(_cur_ctrl_val) / volume_config.max_val;
        if (fabs(volume_config.volume - _t)  >= 0.2)
        { 
            cur_ctrl_val = _cur_ctrl_val;
            volume_config.volume = static_cast<float>(cur_ctrl_val) / volume_config.max_val;
            esp_ai_volume.setVolume(volume_config.volume);
            // DEBUG_PRINTLN(debug, volume_config.volume);
        }
    }

    bool is_use_edge_impulse = wake_up_scheme == "edge_impulse";

    // if (ws_connected && start_ed != "1" && (strcmp(wake_up_config.wake_up_scheme, "edge_impulse") == 0))
    // {
    //     if (inference.buf_ready != 0 && can_voice == "1")
    //     {
    //         wakeup_inference();
    //     }
    // }

    // 内置语音唤醒优化 ing...
    if (is_use_edge_impulse && ws_connected)
    {
        if (inference.buf_ready != 0)
        {
            wakeup_inference();
        }
    }

    else if (wake_up_scheme == "serial" && Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        if (command == String(wake_up_config.str))
        {
            DEBUG_PRINTLN(debug, ("收到串口数据, 唤醒成功"));
            wakeUp();
        }
    }

    else if (wake_up_scheme == "pin_high" || wake_up_scheme == "pin_low")
    {
        int reading = digitalRead(wake_up_config.pin);
        long curTime = millis();
        int target_val = wake_up_scheme == "pin_high" ? 1 : 0;
        if (reading == target_val)
        {
            if ((curTime - lastDebounceTime) > debounceDelay)
            {
                lastDebounceTime = curTime;
                if (prev_state != reading)
                {
                    prev_state = reading;
                    DEBUG_PRINTLN(debug, ("按下了按钮, 唤醒成功"));
                    wakeUp();
                }
            }
        }
        else
        {
            prev_state = reading;
        }
    }

    else if (wake_up_scheme == "asrpro" && Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        String clear_str = cleanString(command);
        if (clear_str == String(wake_up_config.str))
        {
            DEBUG_PRINTLN(debug, ("收到串口数据, 唤醒成功"));
            wakeUp();
        }
        delay(3);
    }

    if (ws_connected && start_ed == "1" && !is_use_edge_impulse && tts_task_id == "")
    {
        size_t diy_wakeup_bytes_read;

        /**
         * 讯飞 iat
         * 建议每次发送音频间隔40ms，
         * 每次发送音频字节数（即java示例demo中的frameSize）为一帧音频大小的整数倍。
         * 未压缩的PCM格式，每次发送音频间隔40ms，每次发送音频字节数1280B；
         *
         * 听写支持识别60s之内的音频。
         * 默认支持50路并发，如需更多并发可提交工单进行咨询
         */
        i2s_read(MIC_i2s_num, (void *)diy_wakeup_sample_buffer, sizeof(diy_wakeup_sample_buffer), &diy_wakeup_bytes_read, portMAX_DELAY);
        esp_ai_webSocket.sendBIN((uint8_t *)diy_wakeup_sample_buffer, diy_wakeup_bytes_read);
        delay(3);
 
    }

    // 上报传感器数据
    long curTime = millis();
    if (curTime - prev_read_to_s_delay_time > read_to_s_delay_time)
    { 
        for (int i = 0; i < digital_read_pins.size(); i++)
        {
            int pin = digital_read_pins[i];
            int reading = digitalRead(pin); 
            digitalReadJSONData["type"] = "digitalRead"; 
            digitalReadJSONData["pin"] = pin;
            digitalReadJSONData["value"] = reading; 
            String sendData = JSON.stringify(digitalReadJSONData); 
            esp_ai_webSocket.sendTXT(sendData);
        }
        for (int i = 0; i < analog_read_pins.size(); i++)
        {
            int pin = analog_read_pins[i];
            int reading = analogRead(pin); 
            analogReadJSONData["type"] = "analogRead"; 
            analogReadJSONData["pin"] = pin;
            analogReadJSONData["value"] = reading; 
            String sendData = JSON.stringify(analogReadJSONData); 
            esp_ai_webSocket.sendTXT(sendData);
        } 

        prev_read_to_s_delay_time = curTime;
    }
}