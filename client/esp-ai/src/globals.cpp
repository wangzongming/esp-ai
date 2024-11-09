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

#include "globals.h"
 
String ESP_AI_VERSION = "2.11.5";
String start_ed = "0";
String can_voice = "1";
String is_send_server_audio_over = "1";
int cur_ctrl_val = 0;
bool ws_connected = false;
String session_id = "";
String tts_task_id = "";

// ing...
int esp_ai_VAD_THRESHOLD = 60;

// 网络状态
String net_status = "0";
// 是否是配网页面链接wifi时错处
String ap_connect_err = "0";

WebSocketsClient esp_ai_webSocket;

I2SStream i2s;
VolumeStream esp_ai_volume(i2s);
EncodedAudioStream esp_ai_dec(&esp_ai_volume, new MP3DecoderHelix()); // Decoding stream

// test... 
// // 上报音频字节流的实际代码
// void dataCallback(uint8_t *mp3_data, size_t len)
// {
//     // Serial.print("mp3 音频生成：");
//     // Serial.print(len);
//     // Serial.println(" bytes");
//     esp_ai_webSocket.sendBIN((uint8_t *)mp3_data, len);
// }
// liblame::MP3EncoderLAME esp_ai_mp3_encoder(dataCallback);
// liblame::AudioInfo esp_ai_mp3_info;

// test...
// uint8_t mp3_sampleBuffer[512];  
// audio_tools::MP3EncoderLAME esp_ai_mp3_encoder;  
// EncodedAudioStream esp_ai_out_stream(&mp3_sampleBuffer, &esp_ai_mp3_encoder);

WebServer esp_ai_server(80);

// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic default_i2s_config_mic = {4, 5, 6};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {16, 17, 15, 16000};
// 默认离线唤醒方案
ESP_AI_wake_up_config default_wake_up_config = {"edge_impulse", 0.95};
// { wifi 账号， wifi 密码 }
ESP_AI_wifi_config default_wifi_config = {"", "", "ESP-AI", ""};
// { ip， port, api_key }
ESP_AI_server_config default_server_config = {"https", "node.espai.fun", 443, "", ""};
// 音量配置 { 输入引脚，输入最大值，默认音量 }
ESP_AI_volume_config default_volume_config = {7, 4096, 0.8, false}; 

inference_t inference;
signed short sampleBuffer[sample_buffer_size];
bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
bool record_status = true;

int16_t mic_sampleBuffer[mic_sample_buffer_size];

String wake_up_scheme = "edge_impulse";


// 灯的数量, 灯带的连接引脚, 使用RGB模式控制ws2812类型灯带，灯带的频率为800KH 
Adafruit_NeoPixel esp_ai_pixels(1, 18, NEO_GRB + NEO_KHZ800); 

/**
 * 生成34位uiud
 */
String generateUUID()
{
    String uuid = "";

    // 生成 UUID 的每部分
    for (int i = 0; i < 8; i++)
    {
        uuid += String(random(0, 16), HEX);
    }
    uuid += "-";
    for (int i = 0; i < 4; i++)
    {
        uuid += String(random(0, 16), HEX);
    }
    uuid += "-4"; // UUID 版本 4
    uuid += String(random(0, 16), HEX);
    uuid += "-";
    uuid += String(random(8, 12), HEX); // UUID 的变种
    for (int i = 0; i < 3; i++)
    {
        uuid += String(random(0, 16), HEX);
    }
    uuid += "-";
    for (int i = 0; i < 12; i++)
    {
        uuid += String(random(0, 16), HEX);
    }
    uuid.toUpperCase();
    return uuid;
}

/**
 * 辅助函数：将字符串写入 EEPROM
 */
void writeStringToEEPROM(int pos, const String &value)
{
    int len = value.length() + 1; // 包括 '\0' 终止符
    for (int i = 0; i < len; i++)
    {
        EEPROM.write(pos + i, value[i]);
    }
    EEPROM.write(pos + len, '\0'); // 确保字符串以 '\0' 结束
}

/**
 * 获取本地存储信息，不传入指定字段时返回全部数据
 * get_local_data("wifi_name"); // oldwang
 */
String get_local_data(const String &field_name = "")
{

    saved_info Info;
    int pos = 0;

    // 辅助函数，读取从 EEPROM 开始位置直到 '\0' 的字符串
    auto readStringFromEEPROM = [&pos]() -> String
    {
        String result = "";
        char c = EEPROM.read(pos);
        while (c != '\0')
        {
            result += c;
            pos++;
            c = EEPROM.read(pos);
        }
        pos++; // 跳过 '\0'
        return result;
    };

    Info.is_ready = readStringFromEEPROM();
    Info.device_id = readStringFromEEPROM();
    Info.wifi_name = readStringFromEEPROM();
    Info.wifi_pwd = readStringFromEEPROM();
    Info.api_key = readStringFromEEPROM();
    Info.ext1 = readStringFromEEPROM();
    Info.ext2 = readStringFromEEPROM();
    Info.ext3 = readStringFromEEPROM();
    Info.ext4 = readStringFromEEPROM();
    Info.ext5 = readStringFromEEPROM();
    // 如果指定了字段名，则返回对应的字段值
    if (field_name == "is_ready")
    {
        return Info.is_ready;
    }
    else if (field_name == "device_id")
    {
        return Info.device_id;
    }
    else if (field_name == "wifi_name")
    {
        return Info.wifi_name;
    }
    else if (field_name == "wifi_pwd")
    {
        return Info.wifi_pwd;
    }
    else if (field_name == "api_key")
    {
        return Info.api_key;
    }
    else if (field_name == "ext1")
    {
        return Info.ext1;
    }
    else if (field_name == "ext2")
    {
        return Info.ext2;
    }
    else if (field_name == "ext3")
    {
        return Info.ext3;
    }
    else if (field_name == "ext4")
    {
        return Info.ext4;
    }
    else if (field_name == "ext5")
    {
        return Info.ext5;
    }

    return "";
}

/**
 * 设置某个数据
 * set_local_data("wifi_name", "oldwang");
 */
void set_local_data(String field_name, String new_value)
{

    // 读取当前保存的信息
    saved_info current_info = {};
    current_info.is_ready = get_local_data("is_ready");
    current_info.device_id = get_local_data("device_id");
    current_info.wifi_name = get_local_data("wifi_name");
    current_info.wifi_pwd = get_local_data("wifi_pwd");
    current_info.api_key = get_local_data("api_key");
    current_info.ext1 = get_local_data("ext1");
    current_info.ext2 = get_local_data("ext2");
    current_info.ext3 = get_local_data("ext3");
    current_info.ext4 = get_local_data("ext4");
    current_info.ext5 = get_local_data("ext5");

    if (field_name == "is_ready")
    {
        current_info.is_ready = new_value;
    }
    else if (field_name == "device_id")
    {
        current_info.device_id = new_value;
    }
    else if (field_name == "wifi_name")
    {
        current_info.wifi_name = new_value;
    }
    else if (field_name == "wifi_pwd")
    {
        current_info.wifi_pwd = new_value;
    }
    else if (field_name == "api_key")
    {
        current_info.api_key = new_value;
    }
    else if (field_name == "ext1")
    {
        current_info.ext1 = new_value;
    }
    else if (field_name == "ext2")
    {
        current_info.ext2 = new_value;
    }
    else if (field_name == "ext3")
    {
        current_info.ext3 = new_value;
    }
    else if (field_name == "ext4")
    {
        current_info.ext4 = new_value;
    }
    else if (field_name == "ext5")
    {
        current_info.ext5 = new_value;
    }
    else
    {
        // 无效的字段名
        Serial.println("[Error] 无效字段：" + field_name);
        return;
    }

    Serial.println("存储字段：" + field_name + "  值：" + new_value);

    // 保存更新后的数据到 EEPROM
    writeStringToEEPROM(0, current_info.is_ready);
    writeStringToEEPROM(current_info.is_ready.length() + 1, current_info.device_id);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1, current_info.wifi_name);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1, current_info.wifi_pwd);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1 + current_info.wifi_pwd.length() + 1, current_info.api_key);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1 + current_info.wifi_pwd.length() + 1 + current_info.api_key.length() + 1, current_info.ext1);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1 + current_info.wifi_pwd.length() + 1 + current_info.api_key.length() + 1 + current_info.ext1.length() + 1, current_info.ext2);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1 + current_info.wifi_pwd.length() + 1 + current_info.api_key.length() + 1 + current_info.ext1.length() + 1 + current_info.ext2.length() + 1, current_info.ext3);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1 + current_info.wifi_pwd.length() + 1 + current_info.api_key.length() + 1 + current_info.ext1.length() + 1 + current_info.ext2.length() + 1 + current_info.ext3.length() + 1, current_info.ext4);
    writeStringToEEPROM(current_info.is_ready.length() + 1 + current_info.device_id.length() + 1 + current_info.wifi_name.length() + 1 + current_info.wifi_pwd.length() + 1 + current_info.api_key.length() + 1 + current_info.ext1.length() + 1 + current_info.ext2.length() + 1 + current_info.ext3.length() + 1 + current_info.ext4.length() + 1, current_info.ext5);
    EEPROM.commit();
    // 确保数据被写入 EEPROM
    delay(100);
}