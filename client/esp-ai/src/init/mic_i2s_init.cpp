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

        #if defined(ARDUINO_XIAO_ESP32S3)
            i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
        #endif

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
    esp_ai_mp3_info.quality = 5;

    esp_ai_mp3_encoder.begin(esp_ai_mp3_info);

    return int(ret);
}
