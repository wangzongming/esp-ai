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
#include "./common.h"
#if defined(ARDUINO_ESP32S3_DEV) 

#define WAKEUP_SCHEME DEFAULT_WAKEUP_SCHEME 
#define BOOT_BUTTON_GPIO GPIO_NUM_0
#define RESET_BTN_GPIO GPIO_NUM_10
#define LIGHTS_GPIO GPIO_NUM_18
#define VOL_GPIO GPIO_NUM_7

#define AUDIO_CODEC_PA_PIN GPIO_NUM_3
#define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_1
#define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_2
#define AUDIO_CODEC_I2C_SPEED 100000
#define AUDIO_CODEC_ES8311_ADDR 0x18
#define AUDIO_CODEC_ES7210_ADDR 0x40


#define SPK_I2S_GPIO_MCLK GPIO_NUM_14
#define SPK_I2S_GPIO_BCLK GPIO_NUM_16
#define SPK_I2S_GPIO_WS GPIO_NUM_17
#define SPK_I2S_GPIO_DIN -1
#define SPK_I2S_GPIO_DOUT GPIO_NUM_15

// #define AUDIO_I2S_GPIO_MCLK GPIO_NUM_14
// #define AUDIO_I2S_GPIO_BCLK GPIO_NUM_16
// #define AUDIO_I2S_GPIO_WS GPIO_NUM_17
// #define AUDIO_I2S_GPIO_DIN -1
// #define AUDIO_I2S_GPIO_DOUT GPIO_NUM_15

#define MIC_I2S_GPIO_MCLK GPIO_NUM_14
#define MIC_I2S_GPIO_BCLK GPIO_NUM_4
#define MIC_I2S_GPIO_WS GPIO_NUM_5
#define MIC_I2S_GPIO_DIN GPIO_NUM_6
#define MIC_I2S_GPIO_DOUT -1
#define I2S_BITS_PER_SAMPLE 16



#endif