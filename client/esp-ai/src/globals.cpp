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
#include <vector>

String ESP_AI_VERSION = "2.46.32";

String esp_ai_start_ed = "0";
bool esp_ai_ws_connected = false;
String esp_ai_session_id = "";
String esp_ai_prev_session_id = "";
String esp_ai_tts_task_id = "";
String esp_ai_status = "";
bool esp_ai_is_listen_model = true;
bool esp_ai_user_has_spoken = false;
bool esp_ai_sleep = false;
bool esp_ai_is_first_send = true;
bool esp_ai_played_connected = false;

// 开始搜集音频
bool esp_ai_start_get_audio = false;
// 开始将音频发往服务
bool esp_ai_start_send_audio = false;
std::vector<uint8_t> *esp_ai_asr_sample_buffer_before;

// 使用串口2进行串口通信
HardwareSerial Esp_ai_serial(2);
Preferences esi_ai_prefs;

// 网络状态
String esp_ai_net_status = "0";
// 是否是配网页面链接wifi时错处
String ap_connect_err = "0";

WebSocketsClient esp_ai_webSocket;

I2SStream esp_ai_spk_i2s;
VolumeStream esp_ai_volume(esp_ai_spk_i2s);
EncodedAudioStream esp_ai_dec(&esp_ai_volume, new MP3DecoderHelix());

void esp_ai_asr_callback(uint8_t *mp3_data, size_t len)
{
    esp_ai_asr_sample_buffer_before->insert(esp_ai_asr_sample_buffer_before->end(), mp3_data, mp3_data + len);
}

liblame::MP3EncoderLAME esp_ai_mp3_encoder(esp_ai_asr_callback);
liblame::AudioInfo esp_ai_mp3_info;

WebServer esp_ai_server(80);

// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic default_i2s_config_mic = {4, 5, 6};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {16, 17, 15, 16000};
// 默认离线唤醒方案
ESP_AI_wake_up_config default_wake_up_config = {"edge_impulse", 0.9};
// { wifi 账号， wifi 密码 }
ESP_AI_wifi_config default_wifi_config = {"", "", "ESP-AI", ""};
// { ip， port, api_key }
ESP_AI_server_config default_server_config = {"https", "node.espai.fun", 443, "", ""};
// 音量配置 { 输入引脚，输入最大值，默认音量 }
ESP_AI_volume_config default_volume_config = {7, 4096, 0.8, false};
// 重置按钮 { 输入引脚，电平： high | low}
ESP_AI_reset_btn_config default_reset_btn_config = {10, "high"};

inference_t inference;
// Set this to true to see e.g. features generated from the raw signal
bool debug_nn = false;
bool esp_ai_wakeup_record_status = true;

int16_t esp_ai_asr_sample_buffer[esp_ai_asr_sample_buffer_size];
int16_t mic_sample_buffer[mic_sample_buffer_size];

// 音频缓存
std::vector<uint8_t> esp_ai_cache_audio_du;
std::vector<uint8_t> esp_ai_cache_audio_greetings;
std::vector<uint8_t> esp_ai_cache_audio_sleep_reply;

long wakeup_time = 0;
long last_silence_time = 0;
long last_not_silence_time = 0;
long last_silence_time_wakeup = 0;
long last_not_silence_time_wekeup = 0;
String play_cache = "";

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

// 获取ESP32的硬件地址（eFuse MAC地址）
String get_device_id()
{
    uint64_t mac = ESP.getEfuseMac();
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (uint8_t)(mac >> 40),
            (uint8_t)(mac >> 32),
            (uint8_t)(mac >> 24),
            (uint8_t)(mac >> 16),
            (uint8_t)(mac >> 8),
            (uint8_t)mac);
    return macStr;
}

/**
 * 获取本地存储信息
 * get_local_data("wifi_name"); // oldwang
 */
String get_local_data(const String &field_name = "")
{
    if (field_name == "device_id")
    {
        return get_device_id();
    }
    esi_ai_prefs.begin("esp-ai-kv", true);
    String val = "";

    if (esi_ai_prefs.isKey(field_name.c_str()))
    {
        val = esi_ai_prefs.getString(field_name.c_str()).c_str();
    }
    esi_ai_prefs.end();
    return val;
}

JSONVar get_local_all_data()
{
    esi_ai_prefs.begin("esp-ai-kv", true);
    JSONVar data;

    String keys_list = esi_ai_prefs.getString("_keys_list_", "");
    if (keys_list != "")
    { 
        int start = 0;
        int end = keys_list.indexOf(',');
        while (end != -1)
        {
            String key = keys_list.substring(start, end);
            String value = esi_ai_prefs.getString(key.c_str());
            data[key] = value;
            start = end + 1;
            end = keys_list.indexOf(',', start);
        }
        // 添加最后一个键值对
        String key = keys_list.substring(start);
        String value = esi_ai_prefs.getString(key.c_str());
        data[key] = value;
    }
 
    esi_ai_prefs.end();
    return data;
}

/**
 * 设置某个数据
 * set_local_data("wifi_name", "oldwang");
 */
void set_local_data(String field_name, String new_value)
{
    esi_ai_prefs.begin("esp-ai-kv", false);

    String keys_list = esi_ai_prefs.getString("_keys_list_", "");
    if (keys_list.indexOf(field_name) == -1)
    {
        if (keys_list != "")
        {
            keys_list += ",";
        }
        keys_list += field_name;
        esi_ai_prefs.putString("_keys_list_", keys_list);
    }

    esi_ai_prefs.putString(field_name.c_str(), new_value.c_str());
    esi_ai_prefs.end();
}

/**
 * 静音检测
 */
bool is_silence(const int16_t *audio_buffer, size_t bytes_read)
{
    if (bytes_read > 0)
    {
        // 计算缓冲区能量
        float energy = 0.0;
        int16_t *samples = (int16_t *)audio_buffer;
        size_t num_samples = bytes_read / sizeof(int16_t);

        for (size_t i = 0; i < num_samples; i++)
        {
            energy += samples[i] * samples[i]; // 信号平方和
        }
        energy /= num_samples; // 平均能量
        // Serial.print("能量: ");
        // Serial.println(energy);

        // 判断是否有语音活动
        if (energy > 10000)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return true;
}

std::vector<int> digital_read_pins;
std::vector<int> analog_read_pins;