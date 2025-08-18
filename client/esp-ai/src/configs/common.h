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
#include <driver/gpio.h>

#define ESP_AI_VERSION "2.87.50"

#define BLE_SERVICE_UUID "BAAD"
#define BLE_CHARACTERISTIC_UUID "F00D"

#define DEFAULT_WAKEUP_SCHEME "pin_high"

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
#define SID_TONE "0000"
#define SID_CONNECTED_SERVER "0001"
#define SID_TONE_CACHE "1000"
#define SID_WAKEUP_REP_CACHE "1001"
#define SID_SLEEP_REP_CACHE "1002"
#define SID_TTS_FN "0010"

/**
 *[会话状态]
 * 00 -> TTS 片段
 * 01 -> TTS 片段（并且是分组最后一个片段）
 * 02 -> 整个回复的TTS最后一组数据，需要继续对话
 * 03 -> 整个回复的TTS最后一组数据，无需继续对话
 */
#define SID_SESSION "00"
#define SID_TTS_CHUNK_END "01"
#define SID_TTS_END_RESTART "02"
#define SID_TTS_END "03"

/**
 * 音频相关配置
 */
#define AUDIO_INPUT_SAMPLE_RATE 16000
#define AUDIO_OUTPUT_SAMPLE_RATE 16000
#define AUDIO_OUTPUT_BITRATE 128

/**
 * OLED 屏幕配置
 */
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y true

#ifndef esp_ai_serial_rx
#if defined(ARDUINO_XIAO_ESP32S3)
#define esp_ai_serial_rx 44
#elif defined(ARDUINO_ESP32C3_DEV)
#define esp_ai_serial_rx WAKEUP_RX
#else
#define esp_ai_serial_rx 12
#endif
#endif

#if defined(CODEC_TYPE_ES8311_NS4150)
#define MIC_i2s_num I2S_NUM_0
#define YSQ_i2s_num I2S_NUM_0
#else
#define MIC_i2s_num I2S_NUM_1
#define YSQ_i2s_num I2S_NUM_0
#endif

/**
 * 各任务大小
 */
#if defined(ARDUINO_ESP32C3_DEV)
#define LIGHTS_TASK_SIZE 1024
#else
#define LIGHTS_TASK_SIZE 1024 * 2
#endif

#define PLAY_AUDIO_TASK_SIZE 1024 * 4
#define ON_REPEATEDLY_CLICK_TASK_SIZE 1024 * 3

#if defined(ARDUINO_ESP32C3_DEV)
#define ON_WAKE_UP_TASK_SIZE 1024 * 2
// #define ON_WAKE_UP_TASK_SIZE 1024 * 4 // debug: 4kb
#else
#define ON_WAKE_UP_TASK_SIZE 1024 * 4
#endif

#define GET_POSITION_TASK_SIZE 1024 * 3
#if defined(ARDUINO_ESP32C3_DEV)
#define VOL_LISTEN_TASK_SIZE 1024 * 2
#else
#define VOL_LISTEN_TASK_SIZE 1024 * 4
#endif
/**
 * 音频缓冲区大小和块大小  
*/
#define AUDIO_BUFFER_SIZE 1024 * 10 // 缓冲区中的总字节数 
#define AUDIO_CHUNK_SIZE 512        // 理想的读/写块大小   
#define AUDIO_COPY_CHUNK_SIZE 512         


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
