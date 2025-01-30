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
#include "speaker_i2s_setup.h"

void ESP_AI::speaker_i2s_setup()
{
    AudioLogger::instance().begin(Serial, AudioLogger::Info);
    // AudioLogger::instance().begin(Serial, AudioLogger::Debug);
    DEBUG_PRINT(debug, F("扬声器采样率："));
    DEBUG_PRINTLN(debug, i2s_config_speaker.sample_rate);
    DEBUG_PRINTLN(debug, "");
    // 配置项文档
    // https://pschatzmann.github.io/arduino-audio-tools/classaudio__tools_1_1_i2_s_config_e_s_p32.html
    // esp32 代码配置处
    // https://github.com/pschatzmann/arduino-audio-tools/blob/9045503daae3b21300ee7bb76c4ad95efe9e1e6c/src/AudioI2S/I2SESP32.h#L186
    auto config = esp_ai_spk_i2s.defaultConfig(TX_MODE);
    config.sample_rate = i2s_config_speaker.sample_rate ? i2s_config_speaker.sample_rate : 16000;
    config.bits_per_sample = 16;
    config.port_no = YSQ_i2s_num; // 这里别和麦克风冲突了，esp32 有两个可用通道
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.i2s_format = I2S_MSB_FORMAT; // 这里最好看看源码中的类型定义
    config.buffer_count = 8;
    config.buffer_size = 1024;
    config.auto_clear = true;
    config.channels = 1;
    config.pin_ws = i2s_config_speaker.ws_io_num;     // LCK
    config.pin_bck = i2s_config_speaker.bck_io_num;   // BCK
    config.pin_data = i2s_config_speaker.data_in_num; // DIN
    esp_ai_spk_i2s.begin(config); 

    esp_ai_dec.begin();
    esp_ai_volume.begin(config); // we need to provide the bits_per_sample and channels
    esp_ai_volume.setVolume(volume_config.volume); 
}