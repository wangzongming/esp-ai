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

#pragma once
#include <string>
#include <stddef.h>
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#include "AudioTools.h"

// 使用 libhelix 对mp3编码
// 要安装插件： https://github.com/pschatzmann/arduino-libhelix
// 注释代码： \Documents\Arduino\libraries\arduino-audio-tool\src\AudioCodecs\CodecMP3Helix.h 85行 --1.x 版本作者已经注释
// #include "AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

// 使用 LAME 对mp3编码
// 要安装插件： https://github.com/pschatzmann/arduino-liblame
// #include "AudioCodecs/CodecMP3LAME.h"
#include "AudioTools/AudioCodecs/CodecMP3LAME.h"

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
// #include "audio/zh/jian_quan_shi_bai.h"
// #include "audio/zh/pei_wang_xin_xi_yi_qing_chu.h"
// #include "audio/zh/qing_lian_jie_fu_wu.h"
// #include "audio/zh/mei_dian_le.h"

// 使用软串口 TX=11，R=12
#ifndef esp_ai_serial_tx
#define esp_ai_serial_tx 11
#endif
#ifndef esp_ai_serial_rx
#define esp_ai_serial_rx 12
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 18
#endif

#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define MIC_i2s_num I2S_NUM_1
#define YSQ_i2s_num I2S_NUM_0

extern HardwareSerial Esp_ai_serial;
extern Preferences esi_ai_prefs;

struct ESP_AI_i2s_config_mic
{
    int bck_io_num;
    int ws_io_num;
    int data_in_num;
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
    // 用户未说话前等待静默时间，默认 5000
    int vad_first;
    // 用户说话后等待静默时间，默认 500
    int vad_course;
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
    // 自定义页面
    String html_str;
    // std::vector<char> html_str;
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
};

extern String esp_ai_net_status;
extern String ap_connect_err;

extern WebSocketsClient esp_ai_webSocket;
extern WebServer esp_ai_server;

extern I2SStream esp_ai_spk_i2s;
extern EncodedAudioStream esp_ai_dec; // Decoding stream
extern VolumeStream esp_ai_volume;

extern liblame::MP3EncoderLAME esp_ai_mp3_encoder;
extern liblame::AudioInfo esp_ai_mp3_info;
void esp_ai_asr_callback(uint8_t *mp3_data, size_t len);

constexpr uint32_t esp_ai_asr_sample_buffer_size = 1280 * 5; // 大约 0.3kb/50ms
extern int16_t esp_ai_asr_sample_buffer[esp_ai_asr_sample_buffer_size];

extern String ESP_AI_VERSION;
extern String esp_ai_start_ed;
extern bool esp_ai_ws_connected;
extern String esp_ai_session_id;
extern String esp_ai_prev_session_id; 
extern String esp_ai_tts_task_id;
extern String esp_ai_status;
extern bool esp_ai_sleep;
extern bool esp_ai_is_first_send;
// 聆听模式
extern bool esp_ai_is_listen_model;
extern bool esp_ai_played_connected;

// 用户已经发话
extern bool esp_ai_user_has_spoken;

// Start collecting audio
extern bool esp_ai_start_get_audio;
// Start sending audio to the service
extern bool esp_ai_start_send_audio;
// circulating register
extern std::vector<uint8_t> *esp_ai_asr_sample_buffer_before;
extern size_t esp_ai_asr_sample_index;

// 音频缓存
extern std::vector<uint8_t> esp_ai_cache_audio_du;
extern std::vector<uint8_t> esp_ai_cache_audio_greetings;
extern std::vector<uint8_t> esp_ai_cache_audio_sleep_reply;

extern long wakeup_time;
extern long last_silence_time;
extern long last_not_silence_time;
extern long last_silence_time_wakeup;
extern long last_not_silence_time_wekeup;
extern String play_cache;

// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
extern ESP_AI_i2s_config_mic default_i2s_config_mic;
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
extern ESP_AI_i2s_config_speaker default_i2s_config_speaker;
// 默认离线唤醒方案
extern ESP_AI_wake_up_config default_wake_up_config;
// { wifi 账号， wifi 密码 }
extern ESP_AI_wifi_config default_wifi_config;
// { ip， port }
extern ESP_AI_server_config default_server_config;
// 音量配置 { 输入引脚，输入最大值，默认音量 }
extern ESP_AI_volume_config default_volume_config;
// 重置按钮 { 输入引脚，输入最大值，默认音量 }
extern ESP_AI_reset_btn_config default_reset_btn_config;

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
extern int16_t mic_sample_buffer[mic_sample_buffer_size];

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
bool is_silence(const int16_t *audio_buffer, size_t bytes_read);

extern std::vector<int> digital_read_pins;
extern std::vector<int> analog_read_pins;