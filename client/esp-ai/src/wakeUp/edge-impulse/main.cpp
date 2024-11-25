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
#include "main.h"
  
/**
 * 关键词模型相关信息和设置
 */
void ESP_AI::wakeup_init()
{ 
}

void ESP_AI::wakeup_inference()
{ 
}

int ESP_AI::microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    // numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
    return 0;
}

bool ESP_AI::microphone_inference_start(uint32_t n_samples)
{
 
    return true;
}

void ESP_AI::microphone_inference_end(void)
{ 
}

bool microphone_inference_record(void)
{ 
    return true;
}

void ESP_AI::capture_samples_wrapper(void *arg)
{ 
}

void ESP_AI::capture_samples(void *arg)
{ 
}

void ESP_AI::audio_inference_callback(uint32_t n_bytes)
{
    
}

int ESP_AI::i2s_deinit(void)
{
    // i2s_driver_uninstall(MIC_i2s_num); // stop & destroy i2s driver
    return 0;
}
// #if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
// #error "Invalid model for current sensor."
// #endif
