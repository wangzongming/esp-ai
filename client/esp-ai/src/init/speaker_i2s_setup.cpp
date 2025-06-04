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
#include "speaker_i2s_setup.h"

void ESP_AI::speaker_i2s_setup()
{
    AudioLogger::instance().begin(Serial, AudioLogger::Error);
    // AudioLogger::instance().begin(Serial, AudioLogger::Info);
    // AudioLogger::instance().begin(Serial, AudioLogger::Debug);

    // 配置项文档
    // https://pschatzmann.github.io/arduino-audio-tools/classaudio__tools_1_1_i2_s_config_e_s_p32.html
    // esp32 代码配置处
    // https://github.com/pschatzmann/arduino-audio-tools/blob/9045503daae3b21300ee7bb76c4ad95efe9e1e6c/src/AudioI2S/I2SESP32.h#L186
    auto config = esp_ai_spk_i2s.defaultConfig(TX_MODE);
    config.sample_rate = i2s_config_speaker.sample_rate ? i2s_config_speaker.sample_rate : 16000;
    config.bits_per_sample = 16;
    config.port_no = YSQ_i2s_num;
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;

    // 这里最好看看源码中的类型定义
    config.i2s_format = I2S_MSB_FORMAT;
    config.buffer_count = 8;
    config.buffer_size = 1024;
    config.auto_clear = true;
    config.channels = 1;
    config.pin_ws = i2s_config_speaker.ws_io_num;     // LCK
    config.pin_bck = i2s_config_speaker.bck_io_num;   // BCK
    config.pin_data = i2s_config_speaker.data_in_num; // DIN
    esp_ai_spk_i2s.begin(config);

    esp_ai_spk_queue.begin();
    esp_ai_volume.begin(config);
    esp_ai_volume.setVolume(volume_config.volume);
    esp_ai_dec.begin(config);
    esp_ai_copier.begin();
}