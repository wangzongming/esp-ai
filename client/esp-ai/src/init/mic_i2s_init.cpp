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
#include "mic_i2s_init.h"

int ESP_AI::mic_i2s_init(uint32_t sampling_rate)
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampling_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_MIC_CHANNEL,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 16,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    i2s_pin_config_t i2s_mic_pins = {
        .bck_io_num = i2s_config_mic.bck_io_num,
        .ws_io_num = i2s_config_mic.ws_io_num,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = i2s_config_mic.data_in_num,
    };

    esp_err_t ret = 0;
    ret = i2s_driver_install(MIC_i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(MIC_i2s_num, &i2s_mic_pins);

    if (ret != ESP_OK)
    {
        Serial.println("[Error] Error in i2s_driver_install");
    }

    if (ret != ESP_OK)
    {
        Serial.println("[Error] Error in i2s_set_pin");
    }

    ret = i2s_zero_dma_buffer(MIC_i2s_num);
    if (ret != ESP_OK)
    {
        Serial.println("[Error] Error in initializing dma buffer with 0");
    }

    // mp3 begin
    esp_ai_mp3_info.channels = 1;
    esp_ai_mp3_info.sample_rate = sampling_rate;
    esp_ai_mp3_info.bits_per_sample = 16;
    esp_ai_mp3_info.quality = 0;  
     
    esp_ai_mp3_encoder.begin(esp_ai_mp3_info); 

    return int(ret);
}
