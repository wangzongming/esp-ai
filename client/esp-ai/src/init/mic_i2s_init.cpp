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
#include "mic_i2s_init.h"

int ESP_AI::mic_i2s_init(uint32_t sampling_rate)
{
    auto i2sConfig = esp_ai_i2s_input.defaultConfig(RX_MODE);
    i2sConfig.bits_per_sample = i2s_config_mic.bits_per_sample ? i2s_config_mic.bits_per_sample : 16;
    i2sConfig.sample_rate = sampling_rate;
    i2sConfig.channels = 1;
    i2sConfig.i2s_format = I2S_LEFT_JUSTIFIED_FORMAT;
    i2sConfig.pin_bck = i2s_config_mic.bck_io_num;
    i2sConfig.pin_ws = i2s_config_mic.ws_io_num;
    i2sConfig.pin_data = i2s_config_mic.data_in_num;
    i2sConfig.port_no = MIC_i2s_num;
    i2sConfig.channel_format = i2s_config_mic.channel_format ? i2s_config_mic.channel_format : I2S_CHANNEL_FMT_ONLY_LEFT;
    i2sConfig.buffer_size = 512;

    // 非常见
    bool unusual = mic_bits_per_sample != 16 && mic_bits_per_sample != 24 && mic_bits_per_sample != 32;
    // 32 位兼容性最高
    if (unusual) i2sConfig.bits_per_sample = 32;
    esp_ai_i2s_input.begin(i2sConfig);

    auto vcfg = esp_ai_mic_volume.defaultConfig();
    vcfg.copyFrom(i2sConfig);
    vcfg.allow_boost = true;
    esp_ai_mic_volume.begin(vcfg);
    if (!unusual) esp_ai_mic_volume.setVolume(10);
    return 0;
}
