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
 
// #include <xiao_ming_tong_xue_inferencing.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// // 如果内存有限，请删除此宏以节省10K RAM
// #define EIDSP_QUANTIZE_FILTERBANK 0
// int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);
// long last_wakeup_time = 0;

/**
 * 关键词模型相关信息和设置
 */
void ESP_AI::wakeup_init()
{
    // // 关键词模型相关信息和设置
    // if (debug)
    // {
    //     ei_printf("Inferencing settings:\n");
    //     ei_printf("\tInterval: ");
    //     ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    //     ei_printf(" ms.\n");
    //     ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    //     ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    //     ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));
    //     ei_printf("2秒后开始连续推理…");
    // }

    // ei_sleep(2000);

    // if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false)
    // {
    //     ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
    //     return;
    // }
}

void ESP_AI::wakeup_inference()
{

//     // 防止连续推理的数据在断时间中出现同结果
//     long cur_wakeup_time = millis();
//     if (cur_wakeup_time - last_wakeup_time < 1500)
//     {
//         return;
//     }

//     inference.buf_ready = 0;

//     signal_t signal;
//     signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
//     signal.get_data = &microphone_audio_signal_get_data;
//     ei_impulse_result_t result = {0};

//     EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
//     if (r != EI_IMPULSE_OK)
//     {
//         ei_printf("ERR: Failed to run classifier (%d)\n", r);
//         return;
//     }

//     if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW))
//     {
//         // print the predictions
//         // ei_printf("\nPredictions ");
//         // ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
//         //           result.timing.dsp, result.timing.classification, result.timing.anomaly);
//         // ei_printf(": \n");
//         float noise = result.classification[0].value;
//         float unknown = result.classification[1].value;
//         float xmtx = result.classification[2].value;
//         ei_printf("=> 得分: ");
//         ei_printf_float(xmtx);
//         // ei_printf("  noise: ");
//         // ei_printf_float(noise);
//         // ei_printf("  unknown: ");
//         // ei_printf_float(unknown);

//         String hx_ok = "0";
//         // 【情况1】高得分
//         if (xmtx >= wake_up_config.threshold)
//         {
//             hx_ok = "1";
//         }
//         else
//         {
//             // 【情况2】关键词与噪音得分两极分化严重
//             // 这种情况也应该唤醒: (关键词得分 - 噪音 - 未知) > 某个阈值(简单设为 0.x)
//             if (((xmtx - noise - unknown) > 0.4) && unknown < 0.3)
//             {
//                 hx_ok = "1";
//             }
//             else
//             {
//                 // 【情况3】关键词得分很低未知分数很高，可能是推理出问题了。需要重新推理一次？
//                 // ...
//             }
//         }
//         if (hx_ok == "1")
//         {
//             if (debug)
//             {
//                 ei_printf("\n[Info] √ 唤醒成功 ");
//                 ei_printf("=> 得分: ");
//                 ei_printf_float(xmtx);
//                 ei_printf("\n");
//             }

//             last_wakeup_time = millis();

//             // 开始录音
//             // digitalWrite(LED_BUILTIN, HIGH);
//             start_ed = "1";

//             JSONVar data;
//             data["type"] = "start";
//             String sendData = JSON.stringify(data);
//             esp_ai_webSocket.sendTXT(sendData);

//             DEBUG_PRINTLN(debug, ("[Info] -> 开始录音"));
//         }

// #if EI_CLASSIFIER_HAS_ANOMALY == 1
//         ei_printf("    anomaly score: ");
//         ei_printf_float(result.anomaly);
//         ei_printf("\n");
// #endif

//         print_results = 0;
//     }
}

int ESP_AI::microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    // numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
    return 0;
}

bool ESP_AI::microphone_inference_start(uint32_t n_samples)
{

    // inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));

    // if (inference.buffers[0] == NULL)
    // {
    //     return false;
    // }

    // inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

    // if (inference.buffers[1] == NULL)
    // {
    //     ei_free(inference.buffers[0]);
    //     return false;
    // }

    // inference.buf_select = 0;
    // inference.buf_count = 0;
    // inference.n_samples = n_samples;
    // inference.buf_ready = 0;

    // if (mic_i2s_init(EI_CLASSIFIER_FREQUENCY))
    // {
    //     ei_printf("Failed to start I2S!");
    // }

    // ei_sleep(100);

    // record_status = true;

    // xTaskCreate(ESP_AI::capture_samples_wrapper, "CaptureSamples", 1024 * 32, (void *)sample_buffer_size, 10, NULL);

    return true;
}

void ESP_AI::microphone_inference_end(void)
{
    // i2s_deinit();
    // ei_free(inference.buffers[0]);
    // ei_free(inference.buffers[1]);
}

bool microphone_inference_record(void)
{
    // bool ret = true;

    // // if (inference.buf_ready == 1)
    // // {
    // //     ei_printf(
    // //         "Error sample buffer overrun. Decrease the number of slices per model window "
    // //         "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
    // //     ret = false;
    // // }

    // while (inference.buf_ready == 0)
    // {
    //     delay(1);
    // }

    // inference.buf_ready = 0;
    return true;
}

void ESP_AI::capture_samples_wrapper(void *arg)
{
    // ESP_AI *instance = static_cast<ESP_AI *>(arg);
    // instance->capture_samples(arg);
}

void ESP_AI::capture_samples(void *arg)
{

    // const int32_t i2s_bytes_to_read = (uint32_t)arg;
    // size_t bytes_read = i2s_bytes_to_read;

    // while (record_status)
    // {

    //     /* read data at once from i2s */
    //     i2s_read(MIC_i2s_num, (void *)mic_sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);
    //     // i2s_read(MIC_i2s_num, (void *)mic_sampleBuffer, sizeof(mic_sampleBuffer), &bytes_read, portMAX_DELAY);

    //     // 调整音量
    //     // adjustVolume((int16_t *)mic_sampleBuffer, sizeof(mic_sampleBuffer), 16);
    //     // 发送给服务端
    //     if (start_ed == "1" && can_voice == "1" && tts_task_id == "")
    //     {
    //         // if (esp_ai_webSocket.isConnected()) { }
    //         esp_ai_webSocket.sendBIN((uint8_t *)mic_sampleBuffer, bytes_read);
    //     }

    //     if (bytes_read <= 0)
    //     {
    //         ei_printf("Error in I2S read : %d", bytes_read);
    //     }
    //     else
    //     {
    //         if (bytes_read < i2s_bytes_to_read)
    //         {
    //             ei_printf("Partial I2S read");
    //         }
            
    //         // 放大、噪音处理、回音处理 ...
    //         // scale the data (otherwise the sound is too quiet)
    //         for (int x = 0; x < i2s_bytes_to_read / 2; x++)
    //         {
    //             // mic_sampleBuffer[x] = (int16_t)(mic_sampleBuffer[x]) * 8;
    //             mic_sampleBuffer[x] = (int16_t)(mic_sampleBuffer[x]) * 16;
    //             // mic_sampleBuffer[x] = (int16_t)(mic_sampleBuffer[x]) * 32;
    //         }

    //         if (record_status)
    //         {
    //             audio_inference_callback(i2s_bytes_to_read);
    //         }
    //         else
    //         {
    //             break;
    //         }
    //     }
    // }
    // vTaskDelete(NULL);
}

void ESP_AI::audio_inference_callback(uint32_t n_bytes)
{
    // for (int i = 0; i < n_bytes >> 1; i++)
    // {
    //     inference.buffers[inference.buf_select][inference.buf_count++] = mic_sampleBuffer[i];

    //     if (inference.buf_count >= inference.n_samples)
    //     {
    //         inference.buf_select ^= 1;
    //         inference.buf_count = 0;
    //         inference.buf_ready = 1;
    //     }
    // }
}

int ESP_AI::i2s_deinit(void)
{
    // i2s_driver_uninstall(MIC_i2s_num); // stop & destroy i2s driver
    return 0;
}
// #if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
// #error "Invalid model for current sensor."
// #endif
