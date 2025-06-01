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

#pragma once
#include <string>
#include <stddef.h>
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

// 使用 v1.0.1 版本
#include "AudioTools.h"

// 使用 libhelix 对mp3解码
// 要安装插件： https://github.com/pschatzmann/arduino-libhelix
// 注释代码： \Documents\Arduino\libraries\arduino-audio-tool\src\AudioCodecs\CodecMP3Helix.h 85行 --1.x 版本作者已经注释
// #include "AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

// 使用 LAME 对mp3编码
// 要安装插件： https://github.com/pschatzmann/arduino-liblame
// #include "AudioCodecs/CodecMP3LAME.h"
// #include "AudioTools/AudioCodecs/CodecMP3LAME.h"

#include <Arduino_JSON.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#include <HardwareSerial.h>
#include <Preferences.h>

#include <esp_adc_cal.h>
#include <driver/adc.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include <DNSServer.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include <HTTPClient.h>

#include "audio/zh/lian_jie_shi_bai.h"
#include "audio/zh/lian_jie_zhong.h"
#include "audio/zh/pei_wang_cheng_gong.h"
#include "audio/zh/qing_pei_wang.h"
#include "audio/zh/san_ci.h"
#include "audio/zh/lian_jie_cheng_gong.h"
#include "audio/zh/fu_wu_lian_jie_zhong.h"
#include "audio/zh/yu_e_bu_zuo.h"
#include "audio/zh/chao_ti_wei_qi_yong.h"
#include "audio/zh/e_du_ka_bu_cun_zai.h"
#include "audio/zh/mei_dian_le.h"
#include "audio/zh/hui_fu_chu_chang.h"
// #include "audio/zh/jian_quan_shi_bai.h"
// #include "audio/zh/pei_wang_xin_xi_yi_qing_chu.h"
// #include "audio/zh/qing_lian_jie_fu_wu.h"

#define BLE_SERVICE_UUID "b09b32d2-1e07-45bb-9319-109d612dead9"
#define BLE_CHARACTERISTIC_UUID "5c1e7878-e48e-4134-af2c-fd64d873125b"

  
extern String SID_TONE;
extern String SID_CONNECTED_SERVER;
extern String SID_TONE_CACHE;
extern String SID_WAKEUP_REP_CACHE;
extern String SID_SLEEP_REP_CACHE;
extern String SID_TTS_END_RESTART;
extern String SID_TTS_END;
extern String SID_TTS_CHUNK_END;
extern String SID_TTS_FN;
extern String SID_SESSION;



// 使用软串口 TX=11，R=12
#ifndef esp_ai_serial_tx
#if defined(ARDUINO_XIAO_ESP32S3)
#define esp_ai_serial_tx 43
#else
#define esp_ai_serial_tx 11
#endif
#endif

#ifndef esp_ai_serial_rx
#if defined(ARDUINO_XIAO_ESP32S3)
#define esp_ai_serial_rx 44
#else
#define esp_ai_serial_rx 12
#endif
#endif

#define MIC_i2s_num I2S_NUM_1
#define YSQ_i2s_num I2S_NUM_0

extern WiFiMulti wifiMulti;
extern HardwareSerial Esp_ai_serial;
extern Preferences esi_ai_prefs;

struct ESP_AI_i2s_config_mic
{
    int bck_io_num;
    int ws_io_num;
    int data_in_num;
    int bits_per_sample;
    int channel_format;
};

struct ESP_AI_i2s_config_speaker
{
    int bck_io_num;
    int ws_io_num;
    int data_in_num;
    int sample_rate;
};

/**
 * 本配置需要调整为宏定义, 下版本处理...
 *
 * 语音唤醒方案：
 * edge_impulse：内置语音唤醒方案 (esp32S3板子支持)
 *       asrpro：天问语音模块唤醒
 *     pin_high：引脚高电平唤醒
 *      pin_low：引脚低电平唤醒
 *     pin_high_listen：引脚高电平聆听(按下对话)
 *      pin_low_listen：引脚低电平聆听(按下对话)
 *       serial：串口字符唤醒
 *       custom：自定义，自行调用 esp_ai.wakeUp() 唤醒
 */
// #define ESP_AI_WAKEUP_SCHEME "edge_impulse"

struct ESP_AI_wake_up_config
{
    // 离线唤醒方案：
    char wake_up_scheme[20];
    // 唤醒阈值 0-1
    float threshold;
    // 引脚唤醒时配置的引脚
    int pin;
    // 串口唤醒时的唤醒字符
    char str[32];
};

// 音量调节配置
struct ESP_AI_volume_config
{
    // 输入引脚，默认 7
    int input_pin;
    // 电位器最大输出值， 一般为 1024 或 4096。不同电位器不同，默认 4096
    int max_val;
    // 音量 0-1，默认 0.5
    float volume;
    // 是否启用电位器引脚
    bool enable;
};

struct ESP_AI_wifi_config
{
    char wifi_name[60];
    char wifi_pwd[60];
    // 热点名字
    char ap_name[30];
    // 自定义配网页面
    String html_str;
    // 配网方式 AP | BLE
    String way;
};

struct ESP_AI_server_config
{
    // 协议： http | https
    char protocol[50];
    // 为 https 协议时，必须是 https://xx 地址
    char ip[100];
    // 为 https 协议时请设置为 443，或者您的 https 端口
    int port;
    // 请求参数，使用 & 符号拼接，eg: name=小明IO&key=123456
    char params[200];
    // 自定义服务路径，注意:后面不要加 /
    // eg: /xxxApi
    char path[100];
};

struct ESP_AI_reset_btn_config
{
    // IO口，默认使用 10。也就是和按钮唤醒使用一个IO
    int pin;
    // 按钮按下时IO口输出为高电平还是低电平："low" | "high"， 默认 'high'
    String power;
};

struct ESP_AI_lights_config
{
    // IO口
    int pin;
};

struct ESP_AI_CONFIG
{
    // debug 模式，输出更多信息
    bool debug;
    // wifi 配置
    ESP_AI_wifi_config wifi_config;
    // 服务配置
    ESP_AI_server_config server_config;
    // 离线唤醒配置
    ESP_AI_wake_up_config wake_up_config;
    // 音量调节配置
    ESP_AI_volume_config volume_config;
    // 麦克风引脚配置
    ESP_AI_i2s_config_mic i2s_config_mic;
    // 扬声器引脚配置
    ESP_AI_i2s_config_speaker i2s_config_speaker;
    // 重置信息按钮配置
    ESP_AI_reset_btn_config reset_btn_config;
    // 灯光配置
    ESP_AI_lights_config lights_config;
};

extern String esp_ai_net_status;
extern String ap_connect_err;

extern WebSocketsClient esp_ai_webSocket;
extern WebServer esp_ai_server;
extern DNSServer esp_ai_dns_server;

extern bool spk_ing;
extern SemaphoreHandle_t audio_mutex;

class BufferPrint : public Print
{
public:
    BufferPrint(BufferRTOS<uint8_t> &buf) : _buffer(buf) {}

    virtual size_t write(uint8_t data) override
    {
        if (!spk_ing)
            return 0;
        if (xSemaphoreTake(audio_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            return _buffer.writeArray(&data, 1);
        }
        return 0;
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        if (!spk_ing)
            return 0; 
        if (xSemaphoreTake(audio_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            size_t result = _buffer.writeArray(buffer, size);
            xSemaphoreGive(audio_mutex);
            return result;
        }
        return 0;
    }

private:
    BufferRTOS<uint8_t> &_buffer;
};
constexpr size_t AUDIO_BUFFER_SIZE = 1024 * 20; // 缓冲区中的总字节数
constexpr size_t AUDIO_CHUNK_SIZE = 1024;       // 理想的读/写块大小
extern I2SStream esp_ai_spk_i2s;
extern EncodedAudioStream esp_ai_dec;
extern MP3DecoderHelix esp_ai_dec_mp3;
extern VolumeStream esp_ai_volume;
extern StreamCopy esp_ai_copier;
extern BufferPrint esp_ai_spk_buffer_print;
extern BufferRTOS<uint8_t> esp_ai_audio_buffer;
extern QueueStream<uint8_t> esp_ai_spk_queue;

#define MIC_SAMPLE_BUFFER_SIZE 1024
extern int mic_bits_per_sample;
extern SemaphoreHandle_t esp_ai_ws_mutex;
class WebsocketStream : public Print
{
public:
    virtual size_t write(uint8_t b) override
    {
        if (!esp_ai_webSocket.isConnected())
        {
            return 1;
        }

        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            esp_ai_webSocket.sendBIN(&b, 1);
            xSemaphoreGive(esp_ai_ws_mutex);
            return 1;
        }

        return 1;
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        if (size == 0 || !esp_ai_webSocket.isConnected())
        {
            return size;
        }

        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (mic_bits_per_sample != 16 && mic_bits_per_sample != 24 && mic_bits_per_sample != 32)
            {
                int limit = 32 - mic_bits_per_sample;
                int samples_read = size / sizeof(int32_t);
                const int32_t *rawBuffer = reinterpret_cast<const int32_t *>(buffer);
                int16_t processedBuffer[MIC_SAMPLE_BUFFER_SIZE];
                for (int i = 0; i < std::min(samples_read, MIC_SAMPLE_BUFFER_SIZE); i++)
                {
                    processedBuffer[i] = (int16_t)(rawBuffer[i] >> limit); // 提取高N位中的有效位
                }
                esp_ai_webSocket.sendBIN((uint8_t *)processedBuffer, samples_read * sizeof(int16_t));
            }
            else
            {
                esp_ai_webSocket.sendBIN(buffer, size);
            }

            xSemaphoreGive(esp_ai_ws_mutex);
            return size;
        }

        return size;
    }
};

extern WebsocketStream ws_stream;
extern I2SStream esp_ai_i2s_input;
extern VolumeStream esp_ai_mic_volume;
extern StreamCopy mic_to_ws_copier;

#define ESP_AI_ASR_SAMPLE_BUFFER_SIZE 16000
extern int16_t *esp_ai_asr_sample_buffer;

extern String ESP_AI_VERSION;
extern String esp_ai_start_ed;
extern bool esp_ai_ws_connected;
extern String esp_ai_session_id;
extern String esp_ai_session_status;
extern String esp_ai_tts_task_id;
extern String esp_ai_status;
extern bool esp_ai_sleep;
extern bool asr_ing;
extern String esp_ai_prev_session_id;

// 聆听模式
extern bool esp_ai_is_listen_model;
extern bool esp_ai_played_connected;

// Start sending audio to the service
extern bool esp_ai_start_send_audio;

// 音频缓存
extern std::vector<uint8_t> esp_ai_cache_audio_du;
extern std::vector<uint8_t> esp_ai_cache_audio_greetings; 

extern long last_silence_time;
extern long last_not_silence_time;
extern long last_silence_time_wakeup;
extern long last_not_silence_time_wekeup;

// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
extern ESP_AI_i2s_config_mic default_i2s_config_mic;
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
extern ESP_AI_i2s_config_speaker default_i2s_config_speaker;
// 默认离线唤醒方案
extern ESP_AI_wake_up_config default_wake_up_config;
// { wifi 账号， wifi 密码, "热点名字", "配网页面HTML", "配网方式：AP | BLE" }
extern ESP_AI_wifi_config default_wifi_config;
// { ip， port }
extern ESP_AI_server_config default_server_config;
// 音量配置 { 输入引脚，输入最大值，默认音量 }
extern ESP_AI_volume_config default_volume_config;
// 重置按钮 { 输入引脚，输入最大值，默认音量 }
extern ESP_AI_reset_btn_config default_reset_btn_config;
extern ESP_AI_lights_config default_lights_config;

extern Adafruit_NeoPixel esp_ai_pixels;

#define DEBUG_PRINT(debug, x) \
    if (debug)                \
    {                         \
        Serial.print(x);      \
    }
#define DEBUG_PRINTLN(debug, x) \
    if (debug)                  \
    {                           \
        Serial.println(x);      \
    }

#define DEBUG_PRINTF(fmt, ...) printf_P(PSTR("[%s][%d]:" fmt "\r\n"), __func__, __LINE__, ##__VA_ARGS__);

/** Audio buffers, pointers and selectors */
typedef struct
{
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

extern inference_t inference;
extern bool debug_nn; // Set this to true to see e.g. features generated from the raw signal
extern bool esp_ai_wakeup_record_status;
constexpr uint32_t mic_sample_buffer_size = 1024 * 3;
extern int16_t *mic_sample_buffer;

extern String wake_up_scheme;

String generateUUID();

/**
 * 处理本地数据存储问题
 * 读取储存的wifi信息等的结构体
 */
typedef struct
{
    String is_ready;  // 仅仅用来保证 EEPROM 处于初始化后的状态
    String device_id; // 设备唯一ID，重置设备时，UID会改变，重启等操作不会改变
    String wifi_name; // 存储的 wifi 名字
    String wifi_pwd;  // 存储的 wifi 密码
    String api_key;   // 存储的 api_key
    String ext1;      // 备用1
    String ext2;      // 备用2
    String ext3;      // 备用3
    String ext4;      // 备用4
    String ext5;      // 备用5
    String ext6;      // 备用
    String ext7;      // 备用
} saved_info;
String get_local_data(const String &field_name);
void set_local_data(String field_name, String new_value);
JSONVar get_local_all_data();
String get_device_id();
void clear_local_all_data();

// 将角度转换为占空比
int angleToDutyCycle(int angle);
// 系统内存初始化
void espai_system_mem_init();
// 打印任务堆栈信息
void print_task_info(void);
// 获取当前是否正在播放
bool mp3_player_is_playing();
// 播放音频
void mp3_player_write(const unsigned char *data, size_t len);
// 立即停止播放
void mp3_player_stop();
// 死等待播放完成
void wait_mp3_player_done();
// 播放内置音频
void play_builtin_audio(const unsigned char *data, size_t len);

extern std::vector<int> digital_read_pins;
extern std::vector<int> analog_read_pins;

String decodeURIComponent(const String &encoded);
String get_ap_name(String ap_name);
extern String ESP_AI_BLE_RD;
extern String ESP_AI_BLE_ERR;
