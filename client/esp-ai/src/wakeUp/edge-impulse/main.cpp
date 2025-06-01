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
#include "main.h"

// 未来的版本中可能移除这些代码
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
    // if (debug)
    // {
    //     ei_printf("Inferencing settings:\n");
    //     ei_printf("\tInterval: ");
    //     ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    //     ei_printf(" ms.\n");
    //     ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    //     ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    //     ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));
    // }

    // if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false)
    // {
    //     ei_printf("[Error]: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
    //     return;
    // }
}

void ESP_AI::wakeup_inference_wrapper(void *arg)
{
    // ESP_AI *instance = static_cast<ESP_AI *>(arg);
    // instance->wakeup_inference();
}

void ESP_AI::wakeup_inference()
{
//     while (true)
//     {
//         if (esp_ai_start_ed == "0")
//         {
//             inference.buf_ready = 0;
//             signal_t signal;
//             signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
//             signal.get_data = &microphone_audio_signal_get_data;
//             ei_impulse_result_t result = {0};

//             EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
//             if (r != EI_IMPULSE_OK)
//             {
//                 ei_printf("ERR: Failed to run classifier (%d)\n", r);
//                 return;
//             }
//             if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW))
//             {
//                 float noise = result.classification[0].value;
//                 float unknown = result.classification[1].value;
//                 float xmtx = result.classification[2].value;

//                 bool wakeup_condition1 = xmtx >= wake_up_config.threshold;
//                 if (wakeup_condition1)
//                 {
//                     DEBUG_PRINT(debug, "\n[Info] √ 唤醒成功 => 得分: ")
//                     DEBUG_PRINT(debug, xmtx)
//                     DEBUG_PRINT(debug, "  unknown: ")
//                     DEBUG_PRINT(debug, unknown)
//                     DEBUG_PRINT(debug, "  noise: ")
//                     DEBUG_PRINTLN(debug, noise)
//                     wakeUp();
//                 }

// #if EI_CLASSIFIER_HAS_ANOMALY == 1
//                 ei_printf("    anomaly score: ");
//                 ei_printf_float(result.anomaly);
//                 ei_printf("\n");
// #endif

//                 print_results = 0;
//             }
//         }
//         vTaskDelay(50 / portTICK_PERIOD_MS);
//     }

//     vTaskDelete(NULL);
// }

// int ESP_AI::microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
// {
//     numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
//     return 0;
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

    // esp_ai_wakeup_record_status = true;

    // xTaskCreate(ESP_AI::capture_samples_wrapper, "capture_samples", 1024 * 6, this, 10, NULL);
    // return true;
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

    // if (inference.buf_ready == 1)
    // {
    //     ei_printf("[Error] sample buffer overrun. Decrease the number of slices per model window "
    //               "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
    //     ret = false;
    // }

    // while (inference.buf_ready == 0)
    // {
    //     delay(1);
    // }

    // inference.buf_ready = 0;
    // return true;
}

void ESP_AI::capture_samples_wrapper(void *arg)
{
    // ESP_AI *instance = static_cast<ESP_AI *>(arg);
    // instance->capture_samples();
}

void ESP_AI::capture_samples()
{
    // const int32_t i2s_bytes_to_read = mic_sample_buffer_size;
    // size_t bytes_read = i2s_bytes_to_read;
    // bool _is_silence = true;

    // while (esp_ai_wakeup_record_status)
    // {

    //     i2s_read(MIC_i2s_num, (void *)mic_sample_buffer, i2s_bytes_to_read, &bytes_read, 100);
    //     if (esp_ai_ws_connected && esp_ai_start_send_audio)
    //     {

    //         size_t sample_count = bytes_read / sizeof(mic_sample_buffer[0]);
    //         int gain_factor = 16;
    //         for (size_t i = 0; i < sample_count; i++)
    //         {
    //             mic_sample_buffer[i] *= gain_factor;
    //         }
    //         esp_ai_webSocket.sendBIN((uint8_t *)mic_sample_buffer, bytes_read);
    //     }

    //     if (bytes_read <= 0)
    //     {
    //         ei_printf("Error in I2S read : %d", bytes_read);
    //     }

    //     if (bytes_read < i2s_bytes_to_read)
    //     {
    //         ei_printf("Partial I2S read");
    //     }
    //     for (int x = 0; x < i2s_bytes_to_read / 2; x++)
    //     {
    //         mic_sample_buffer[x] = (int16_t)(mic_sample_buffer[x]) * 16;
    //     }

    //     if (esp_ai_wakeup_record_status)
    //     {
    //         audio_inference_callback(i2s_bytes_to_read);
    //     }
    //     else
    //     {
    //         break;
    //     }
    //     vTaskDelay(50 / portTICK_PERIOD_MS);
    // }
    // vTaskDelete(NULL);
}

void ESP_AI::audio_inference_callback(uint32_t n_bytes)
{
    // for (int i = 0; i < n_bytes >> 1; i++)
    // {
    //     inference.buffers[inference.buf_select][inference.buf_count++] = mic_sample_buffer[i];

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
    // return 0;
}
// #if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
// #error "Invalid model for current sensor."
// #endif
