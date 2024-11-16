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

// 音频流播放插件
#include "AudioTools.h"

// 使用 libhelix 对mp3编码
// 要安装插件： https://github.com/pschatzmann/arduino-libhelix
// 注释代码： \Documents\Arduino\libraries\arduino-audio-tool\src\AudioCodecs\CodecMP3Helix.h 85行
#include "AudioCodecs/CodecMP3Helix.h"   

// 使用 LAME 对mp3编码  test...
// 要安装插件： https://github.com/pschatzmann/arduino-liblame
#include "AudioCodecs/CodecMP3LAME.h"  
// #include "MP3EncoderLAME.h"

#include <Arduino_JSON.h>
#include <WebServer.h>
#include <EEPROM.h> 
#include <Adafruit_NeoPixel.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 18 
#endif

// 大多数麦克风可能默认为左通道，但可能需要将L/R引脚绑低
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define MIC_i2s_num I2S_NUM_1
#define YSQ_i2s_num I2S_NUM_0

// test...
// using namespace liblame;

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
    // 输入引脚，默认 34
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
    String  html_str; 
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
};

extern String net_status;
extern String ap_connect_err;

extern WebSocketsClient esp_ai_webSocket;
extern WebServer esp_ai_server;

extern I2SStream i2s; 
extern EncodedAudioStream esp_ai_dec; // Decoding stream 
extern VolumeStream esp_ai_volume;
 

extern String ESP_AI_VERSION; 
extern String start_ed; 
extern String can_voice; 
extern String is_send_server_audio_over; 
extern int cur_ctrl_val; 
extern bool ws_connected; 
extern String session_id;
extern String tts_task_id;
extern int esp_ai_VAD_THRESHOLD;

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
constexpr uint32_t sample_buffer_size = 2048; //  1024 2048 √ 4096
extern signed short sampleBuffer[sample_buffer_size];
extern bool debug_nn; // Set this to true to see e.g. features generated from the raw signal 
extern bool record_status;

// mic setting
constexpr uint32_t mic_sample_buffer_size = 512;  
extern int16_t mic_sampleBuffer[mic_sample_buffer_size];

extern String wake_up_scheme;

String generateUUID();

/**
 * 处理本地数据存储问题
 * 读取储存的wifi信息等的结构体
*/
typedef struct
{
    String is_ready;    // 仅仅用来保证 EEPROM 处于初始化后的状态
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
 
extern std::vector<int> digital_read_pins;
extern std::vector<int> analog_read_pins;