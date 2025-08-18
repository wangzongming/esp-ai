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

/**
 * C3 配置文件，为了硬件更好的兼容，避免造成市面上IO混乱，使用开源项目 xmini-c3 相同 IO
 * 
 ****** C3 和 S3 的区别： *******
 * 1. 不支持同时开启 AP 和 蓝牙配网
 * 2. 不支持记忆多个 wifi
 * 3. 不支持传感器控制配置
 * 
 */
#pragma once
#include <driver/gpio.h>
#include "./common.h"
#if defined(ARDUINO_ESP32C3_DEV)
 
#define LITTLE_ROM // 可用内存 <= 300KB.
#define CODEC_TYPE_ES8311_NS4150

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_10
#define AUDIO_I2S_GPIO_WS GPIO_NUM_6
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_8
#define AUDIO_I2S_GPIO_DIN GPIO_NUM_7 // MIC
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_5

#define AUDIO_CODEC_PA_PIN GPIO_NUM_11
#define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_3
#define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_4 
#define AUDIO_CODEC_I2C_SPEED 100000
#define AUDIO_CODEC_ES8311_ADDR 0x18

#define BUILTIN_LED_GPIO GPIO_NUM_2
#define BOOT_BUTTON_GPIO GPIO_NUM_9

#define WAKEUP_RX GPIO_NUM_20
#define WAKEUP_TX GPIO_NUM_21

// #define WAKEUP_SCHEME DEFAULT_WAKEUP_SCHEME
#define WAKEUP_SCHEME "pin_low"


// ESP-AI 蝴蝶结特殊宏定义
// #define IS_BOWKNOT
// #define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_12
// #define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_13 

#endif