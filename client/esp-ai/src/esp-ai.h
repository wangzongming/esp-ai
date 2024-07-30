#pragma once

#include <string>
#include <stddef.h> // 包含NULL定义
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
// 音频流播放插件
#include "AudioTools.h"

// // 模型插件
// #include <xiao_ming_tong_xue_inferencing.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

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
struct ESP_AI_wake_up_config
{
    // 离线唤醒方案："edge_impulse" | "diy"，为 "diy" 时可调用 esp_ai.wakeUp() 方法进行唤醒
    char wake_up_scheme[20];
    // 唤醒阈值 0-1
    float threshold;
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
};

struct ESP_AI_wifi_config
{
    char wifi_name[20];
    char wifi_pwd[20];
};

struct ESP_AI_server_config
{
    char ip[16];
    int port;
    char params[512];
};

struct ESP_AI_CONFIG
{
    // 麦克风引脚配置
    ESP_AI_i2s_config_mic i2s_config_mic;
    // 扬声器引脚配置
    ESP_AI_i2s_config_speaker i2s_config_speaker;
    // wifi 配置
    ESP_AI_wifi_config wifi_config;
    // 服务配置
    ESP_AI_server_config server_config;
    // 离线唤醒配置
    ESP_AI_wake_up_config wake_up_config;
    // 音量调节配置
    ESP_AI_volume_config volume_config;
    // debug 模式，输出更多信息
    bool debug;
};

class ESP_AI
{
public:
    ESP_AI();
    void begin(ESP_AI_CONFIG config);
    void loop();
    bool wifiIsConnected();
    std::string localIP();
    // 唤醒
    void wakeUp();
    // 设置音量 0-100
    void setVolume(int volume);
    /**
     * 接收到控制命令后的后调
     * command_id 命令id
     * data       其他数据
     **/ 
    void onEvent(void (*func)(String command_id, String data));

private:
    ESP_AI_i2s_config_mic i2s_config_mic;
    ESP_AI_i2s_config_speaker i2s_config_speaker;
    ESP_AI_wifi_config wifi_config;
    ESP_AI_server_config server_config;
    ESP_AI_wake_up_config wake_up_config;
    ESP_AI_volume_config volume_config;
    bool debug;
    // void (*onEventCb)(char command_id, char data) = nullptr; // 初始化为nullptr
    void (*onEventCb)(String command_id, String data) = nullptr; // 初始化为nullptr

    void speaker_i2s_setup();
    void adjustVolume(int16_t *buffer, size_t length, float volume);
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    int mic_i2s_init(uint32_t sampling_rate);
    bool microphone_inference_start(uint32_t n_samples); 
    void microphone_inference_end(void); 
    void capture_samples(void* arg);
    static void capture_samples_wrapper(void* arg);

    void audio_inference_callback(uint32_t n_bytes);
    int i2s_deinit(void);
};
 
