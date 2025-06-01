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

#include "globals.h"
#include <vector>

String ESP_AI_VERSION = "2.84.43";

String esp_ai_start_ed = "0";       // AST 服务推理中
bool esp_ai_ws_connected = false;   // ws 服务已经连接成功
String esp_ai_session_id = "";      // 会话ID，用于判断是否应该丢弃会话
String esp_ai_session_status = "";      // 会话状态 
String esp_ai_tts_task_id = "";     // TTS chunk ID
String esp_ai_status = "";          // 设备状态
bool esp_ai_is_listen_model = true; // 是否为按住对话模式, 这个模式下按住按钮才会进行聆听，放开后进行LLM推理 
bool esp_ai_sleep = false;            // 是否为休眠状态，开发中...
bool esp_ai_played_connected = false; // 是否已经播放过 服务连接成功 的提示语了
bool asr_ing = false;                 // 硬件是否正在进行收音
String esp_ai_prev_session_id = "";   // 上一轮会话ID


/** 
 * [特定的数据帧]
 * 0000 -> 嘟提示音数据
 * 0001 -> 服务连接成功提示语
 * 0010 -> 调用 .tts() 方法时播放的音频
 * 1000 -> 提示音缓存数据
 * 1001 -> 唤醒问候语缓存数据
 * 1002 -> 休息时回复缓存数据
 * xxxx -> 正常会话ID  
 */
String SID_TONE = "0000";
String SID_CONNECTED_SERVER = "0001";
String SID_TONE_CACHE = "1000";
String SID_WAKEUP_REP_CACHE = "1001";
String SID_SLEEP_REP_CACHE = "1002";
String SID_TTS_FN = "0010";

/** 
 *[会话状态]
 * 00 -> TTS 片段
 * 01 -> TTS 片段（并且是分组最后一个片段）
 * 02 -> 整个回复的TTS最后一组数据，需要继续对话
 * 03 -> 整个回复的TTS最后一组数据，无需继续对话
 */
String SID_SESSION = "00";
String SID_TTS_CHUNK_END = "01";
String SID_TTS_END_RESTART = "02";
String SID_TTS_END = "03";

// 开始将音频发往服务
bool esp_ai_start_send_audio = false;

// 尝试连接多个 wifi
WiFiMulti wifiMulti;
// 使用串口2进行串口通信
HardwareSerial Esp_ai_serial(2);
Preferences esi_ai_prefs;

// 网络状态
String esp_ai_net_status = "0";
// 是否是配网页面链接wifi时错处
String ap_connect_err = "0";

WebSocketsClient esp_ai_webSocket;
SemaphoreHandle_t esp_ai_ws_mutex;

bool spk_ing = false;
SemaphoreHandle_t audio_mutex = xSemaphoreCreateMutex();
I2SStream esp_ai_spk_i2s;
BufferRTOS<uint8_t> esp_ai_audio_buffer(AUDIO_BUFFER_SIZE, AUDIO_CHUNK_SIZE);
QueueStream<uint8_t> esp_ai_spk_queue(esp_ai_audio_buffer);
VolumeStream esp_ai_volume(esp_ai_spk_i2s);
EncodedAudioStream esp_ai_dec(&esp_ai_volume, new MP3DecoderHelix());
StreamCopy esp_ai_copier(esp_ai_dec, esp_ai_spk_queue);
BufferPrint esp_ai_spk_buffer_print(esp_ai_audio_buffer);

WebsocketStream ws_stream;
I2SStream esp_ai_i2s_input;
VolumeStream esp_ai_mic_volume(esp_ai_i2s_input);
StreamCopy mic_to_ws_copier(ws_stream, esp_ai_mic_volume, 1024);

WebServer esp_ai_server(80);
DNSServer esp_ai_dns_server;

#if defined(ARDUINO_XIAO_ESP32S3)
// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic default_i2s_config_mic = {I2S_PIN_NO_CHANGE, 42, 41};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {2, 3, 1, 16000};
// 重置按钮 { 输入引脚，电平： high | low}
ESP_AI_reset_btn_config default_reset_btn_config = {9, "high"};
// 灯光配置
ESP_AI_lights_config default_lights_config = {18};
#else
// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num, i2s_bits_per_sample, 声道选择 }
ESP_AI_i2s_config_mic default_i2s_config_mic = {4, 5, 6, 16, I2S_CHANNEL_FMT_ONLY_LEFT};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {16, 17, 15, 16000};
// 重置按钮 { 输入引脚，电平： high | low}
ESP_AI_reset_btn_config default_reset_btn_config = {10, "high"};
// 灯光配置
ESP_AI_lights_config default_lights_config = {4};
#endif

// 音量配置 { 输入引脚，输入最大值，默认音量 }
ESP_AI_volume_config default_volume_config = {7, 4096, 1, false};

// 默认离线唤醒方案
ESP_AI_wake_up_config default_wake_up_config = {"edge_impulse", 0.9};
// { wifi 账号， wifi 密码, "热点名字", "配网页面HTML", "配网方式：AP | BLE" }
ESP_AI_wifi_config default_wifi_config = {"", "", "ESP-AI", "", "AP"};
// { ip， port, api_key }
ESP_AI_server_config default_server_config = {"https", "node.espai2.fun", 443, "", ""};

inference_t inference;
// Set this to true to see e.g. features generated from the raw signal
bool debug_nn = false;
bool esp_ai_wakeup_record_status = true;

int mic_bits_per_sample = 16;
int16_t *esp_ai_asr_sample_buffer = NULL;
int16_t *mic_sample_buffer = NULL;

// 音频缓存
std::vector<uint8_t> esp_ai_cache_audio_du;
std::vector<uint8_t> esp_ai_cache_audio_greetings; 

long last_silence_time = 0;
long last_not_silence_time = 0;
long last_silence_time_wakeup = 0;
long last_not_silence_time_wekeup = 0;

String wake_up_scheme = "edge_impulse";

#if defined(ARDUINO_XIAO_ESP32S3)
Adafruit_NeoPixel esp_ai_pixels(1, 4, NEO_GRB + NEO_KHZ800);
#else
// 灯的数量, 灯带的连接引脚, 使用RGB模式控制ws2812类型灯带，灯带的频率为800KH
Adafruit_NeoPixel esp_ai_pixels(1, 18, NEO_GRB + NEO_KHZ800);
#endif

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
    return WiFi.macAddress();
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
    esi_ai_prefs.begin("esp-ai-kv", false);
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
    esi_ai_prefs.begin("esp-ai-kv", false);
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
 * 清除本地存储的所有信息
 */
void clear_local_all_data()
{
    esi_ai_prefs.clear();
}

/**
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
 * 将角度转换为占空比
 */
int angleToDutyCycle(int angle)
{
    // 舵机的脉冲宽度范围通常在 0.5ms - 2.5ms
    // 对于 50Hz 的频率，周期为 20ms
    // 分辨率为 10 位时，最大值为 1023
    int dutyCycle = map(angle, 0, 180, 26, 123);
    return dutyCycle;
}

std::vector<int> digital_read_pins;
std::vector<int> analog_read_pins;

String decodeURIComponent(const String &encoded)
{
    String decoded = "";
    for (int i = 0; i < encoded.length(); i++)
    {
        if (encoded[i] == '%')
        {
            if (i + 2 < encoded.length())
            {
                // 提取 % 后面的两位十六进制字符
                String hexStr = encoded.substring(i + 1, i + 3);
                char decodedChar = (char)strtol(hexStr.c_str(), NULL, 16);
                decoded += decodedChar;
                i += 2; // 跳过已经处理的两位十六进制字符
            }
            else if (encoded[i] == '+')
            {
                // 处理 + 符号，它通常代表空格
                decoded += ' ';
            }
            else

            {
                // 普通字符直接添加到结果中
                decoded += encoded[i];
            }
        }
        return decoded;
    }
}

String get_ap_name(String ap_name)
{
    String device_id = get_local_data("device_id");
    String lastTwoChars = device_id.substring(device_id.length() - 5);
    lastTwoChars.replace(":", "");
    String name = ap_name.length() > 0 ? ap_name : "ESP-AI:" + lastTwoChars;
    return name;
}

// 接收到的客户端蓝牙数据
String ESP_AI_BLE_RD;
String ESP_AI_BLE_ERR = "";

void espai_system_mem_init()
{
    if (psramFound())
    {
        Serial.println("[Info] PSRAM 检测成功!");
        Serial.print("[Info] Total PSRAM: ");
        Serial.println(ESP.getPsramSize());
        Serial.print("[Info] Free PSRAM: ");
        Serial.println(ESP.getFreePsram());
    }
    else
    {
        Serial.println(F("[Error] PSRAM 无效，请确保您使用的是 esp32s3 开发板，并且开启了【设置/PSRAM/OPI PSRAM】"));
        Serial.println(F("[Error] 注意分区方案需要选择： 16MB Flash(3MB APP/9.9MB FATFS)"));
    }

    if (psramFound())
    {
        esp_ai_asr_sample_buffer = (int16_t *)ps_malloc(ESP_AI_ASR_SAMPLE_BUFFER_SIZE * sizeof(int16_t));
        mic_sample_buffer = (int16_t *)ps_malloc(mic_sample_buffer_size * sizeof(int16_t));
    }
    else
    {
        esp_ai_asr_sample_buffer = (int16_t *)malloc(ESP_AI_ASR_SAMPLE_BUFFER_SIZE * sizeof(int16_t));
        mic_sample_buffer = (int16_t *)malloc(mic_sample_buffer_size * sizeof(int16_t));
    }

    if (NULL == esp_ai_asr_sample_buffer || NULL == mic_sample_buffer)
    {
        Serial.println(F("[Error] 内存分配失败，请检查内存是否足够"));
        while (1)
            ;
    }
}

void print_task_info(void)
{
    // 获取当前任务的句柄
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();
    // 获取当前任务的名字
    const char *taskName = pcTaskGetName(currentTaskHandle);
    // 获取当前任务的堆栈高水位标记
    UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(currentTaskHandle);
    Serial.printf("任务名称: %s 剩余堆栈: %d bytes\n", taskName, stackHighWaterMark);
}

bool mp3_player_is_playing()
{
    return esp_ai_spk_queue.available();
}

void mp3_player_write(const unsigned char *data, size_t len)
{
    if (len > 0)
    {
        esp_ai_spk_buffer_print.write(data, len);
    }
}

void mp3_player_stop()
{
    spk_ing = false;
    // 等待写入任务停止
    vTaskDelay(pdMS_TO_TICKS(100));
    // 进入临界区
    xSemaphoreTake(audio_mutex, portMAX_DELAY);
    esp_ai_spk_i2s.flush();      // 停止 I2S 输出
    esp_ai_spk_queue.flush();    // 清空 QueueStream
    esp_ai_audio_buffer.reset(); // 重置 ring buffer
    xSemaphoreGive(audio_mutex);
}

void wait_mp3_player_done()
{
    while (mp3_player_is_playing())
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        Serial.printf("*");
    }
    Serial.printf("\n");
    vTaskDelay(pdMS_TO_TICKS(100));
}

void play_builtin_audio(const unsigned char *data, size_t len)
{
    spk_ing = true; 
    size_t remaining = len;
    const unsigned char *current = data;

    while ((remaining > 0) && spk_ing)
    {
        size_t chunk_size = (remaining < AUDIO_CHUNK_SIZE) ? remaining : AUDIO_CHUNK_SIZE;
        mp3_player_write(current, chunk_size);
        current += chunk_size;
        remaining -= chunk_size;
    }
    spk_ing = false;
}