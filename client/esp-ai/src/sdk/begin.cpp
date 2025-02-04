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

#include "begin.h"

void ESP_AI::begin(ESP_AI_CONFIG config)
{
    delay(100);

    esp_ai_asr_sample_buffer_before = new std::vector<uint8_t>();
    esp_ai_asr_sample_buffer_before->reserve(48000 * sizeof(int16_t)); // 预分配

    // 参数包括串行通信的波特率、串行模式、使用的 RX 引脚和 TX 引脚。
    Esp_ai_serial.begin(115200, SERIAL_8N1, esp_ai_serial_rx, esp_ai_serial_tx);

    if (psramFound())
    {
        Serial.println("[Info] PSRAM 检测成功!");
        Serial.print("[Info] Total PSRAM: ");
        Serial.println(ESP.getPsramSize());
        Serial.print("[Info] Free PSRAM: ");
        Serial.println(ESP.getFreePsram());
    }
    else
    {
        Serial.println("PSRAM 无效，请确保您使用的是 esp32s3 开发板，并且开启了【设置/PSRAM/OPI PSRAM】");
        Serial.println("注意分区方案需要选择： 16MB Flash(3MB APP/9.9MB FATFS)");
    }

    Serial.print("[Info] 剩余运行内存:");
    Serial.print(esp_get_free_heap_size() / 1024 / 1024);
    Serial.println(" mb");

    if (config.i2s_config_mic.bck_io_num)
    {
        i2s_config_mic = config.i2s_config_mic;
    }
    if (config.i2s_config_speaker.bck_io_num)
    {
        i2s_config_speaker = config.i2s_config_speaker;
    }

    if (strcmp(config.wifi_config.wifi_name, "") != 0 || strcmp(config.wifi_config.ap_name, "") != 0 || config.wifi_config.html_str != "")
    {
        wifi_config = config.wifi_config;
    }
    if (strcmp(config.server_config.ip, "") != 0)
    {
        server_config = config.server_config;
    }
    if (config.volume_config.input_pin)
    {
        volume_config = config.volume_config;
    }
    if (config.debug)
    {
        debug = config.debug;
    }

    // reset_btn_config
    if (config.reset_btn_config.pin)
    {
        reset_btn_config = config.reset_btn_config;
    }
    if (reset_btn_config.power == "high")
    {
        pinMode(reset_btn_config.pin, INPUT_PULLDOWN);
    }
    else if (reset_btn_config.power == "low")
    {
        pinMode(reset_btn_config.pin, INPUT_PULLUP);
    }

    if (strcmp(config.wake_up_config.wake_up_scheme, "") != 0)
    {
        wake_up_config = config.wake_up_config;
        wake_up_scheme = String(wake_up_config.wake_up_scheme);
        if (wake_up_scheme == "pin_high")
        {
            pinMode(wake_up_config.pin, INPUT_PULLDOWN);
        }
        else if (wake_up_scheme == "pin_low")
        {
            pinMode(wake_up_config.pin, INPUT_PULLUP);
        }

        if (!wake_up_config.vad_first)
        {
            wake_up_config.vad_first = 5000;
        }
        if (!wake_up_config.vad_course)
        {
            wake_up_config.vad_course = 1500;
        }
    }
    esp_ai_is_listen_model = (wake_up_scheme == "pin_high_listen" || wake_up_scheme == "pin_low_listen");

    // 将按钮默认拉低
    if (wake_up_scheme != "pin_high" && wake_up_scheme != "pin_low" && config.reset_btn_config.power != "high" && config.reset_btn_config.power != "low")
    {
        pinMode(wake_up_config.pin, INPUT_PULLDOWN);
    }
 

    // ws2812
    esp_ai_pixels.begin();
    esp_ai_pixels.setBrightness(100); // 亮度设置
    esp_ai_pixels.clear();            // 将所有像素颜色设置为“off”
    esp_ai_pixels.show();             // Initialize all pixels to 'off'
 

    speaker_i2s_setup();
 

    // 内置状态处理
    status_change("0");
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "0";
        onNetStatusCb("0");
    }

    String loc_device_id = get_device_id();
    String loc_wifi_name = get_local_data("wifi_name");
    String loc_wifi_pwd = get_local_data("wifi_pwd");
    String loc_api_key = get_local_data("api_key");
    String loc_ext1 = get_local_data("ext1");
    String loc_ext2 = get_local_data("ext2");
    String loc_ext3 = get_local_data("ext3");
    String loc_ext4 = get_local_data("ext4");
    String loc_ext5 = get_local_data("ext5");
    String loc_ext6 = get_local_data("ext6");
    String loc_ext7 = get_local_data("ext7");
    DEBUG_PRINTLN(debug, F("==================== Local Data ===================="));
    DEBUG_PRINT(debug, F("[Info] loc_device_id: "));
    DEBUG_PRINTLN(debug, loc_device_id);
    DEBUG_PRINT(debug, F("[Info] loc_wifi_name: "));
    DEBUG_PRINTLN(debug, loc_wifi_name);
    DEBUG_PRINT(debug, F("[Info] loc_wifi_pwd: "));
    DEBUG_PRINTLN(debug, loc_wifi_pwd);
    DEBUG_PRINT(debug, F("[Info] loc_api_key: "));
    DEBUG_PRINTLN(debug, loc_api_key);
    DEBUG_PRINT(debug, F("[Info] loc_ext1: "));
    DEBUG_PRINTLN(debug, loc_ext1);
    DEBUG_PRINT(debug, F("[Info] loc_ext2: "));
    DEBUG_PRINTLN(debug, loc_ext2);
    DEBUG_PRINT(debug, F("[Info] loc_ext3: "));
    DEBUG_PRINTLN(debug, loc_ext3);
    DEBUG_PRINT(debug, F("[Info] loc_ext4: "));
    DEBUG_PRINTLN(debug, loc_ext4);
    DEBUG_PRINT(debug, F("[Info] loc_ext5: "));
    DEBUG_PRINTLN(debug, loc_ext5);
    DEBUG_PRINT(debug, F("[Info] loc_ext6: "));
    DEBUG_PRINTLN(debug, loc_ext6);
    DEBUG_PRINT(debug, F("[Info] loc_ext7: "));
    DEBUG_PRINTLN(debug, loc_ext7);
    DEBUG_PRINTLN(debug, F("====================================================="));
 

    xTaskCreate(ESP_AI::on_repeatedly_click_wrapper, "on_repeatedly_click", 1024 * 4, this, 1, NULL);
 

    DEBUG_PRINTLN(debug, ("==================== Connect WIFI ===================="));
    String _wifi_name = loc_wifi_name;
    String _wifi_pwd = loc_wifi_pwd;
    ap_connect_err = "0";
    if (_wifi_name == "")
    {
        _wifi_name = wifi_config.wifi_name;
    }
    if (_wifi_pwd == "")
    {
        _wifi_pwd = wifi_config.wifi_pwd;
    }
 

    if (_wifi_name == "")
    {
        DEBUG_PRINTLN(debug, F("\n[Info] 没有wifi信息，请配网"));
        open_ap();
        return;
    }
 
    esp_ai_dec.write(lian_jie_zhong, lian_jie_zhong_len);
 

    DEBUG_PRINTLN(debug, "[Info] wifi name: " + _wifi_name);
    DEBUG_PRINTLN(debug, "[Info] wifi pwd: " + _wifi_pwd);

    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifi_name, _wifi_pwd);

    DEBUG_PRINT(debug, F("[Info] connect wifi ing..")); 

    int connect_count = 0;
    int try_count = 30;
    while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
    {
        connect_count++;
        // 内置状态处理
        status_change("0_ing");
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing");
        }
        DEBUG_PRINT(debug, ".");
        delay(250);
        // 内置状态处理
        status_change("0_ing_after");
        // 设备状态回调
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing_after");
        }
        delay(250);
        if (connect_count > try_count)
        {
            DEBUG_PRINTLN(debug, ("\n[Error] 连接WIFI失败，请重新配网"));
            open_ap();
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        esp_ai_dec.write(lian_jie_shi_bai, lian_jie_shi_bai_len);
        return;
    } 

    esp_ai_dec.write(lian_jie_cheng_gong, lian_jie_cheng_gong_len);
    esp_ai_dec.write(fu_wu_lian_jie_zhong, fu_wu_lian_jie_zhong_len);
 
    esp_ai_played_connected = false;
    // 内置状态处理
    status_change("2");
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "2";
        onNetStatusCb("2");
    }

    WiFi.setSleep(false);
 

    DEBUG_PRINTLN(debug, "");
    DEBUG_PRINT(debug, F("[Info] wifi 连接成功。"));
    String ip_str = WiFi.localIP().toString();
    if (onConnectedWifiCb != nullptr)
    {
        onConnectedWifiCb(ip_str);
    }

    DEBUG_PRINTLN(debug, ("==============================================="));
 

    if (mic_i2s_init(16000))
    {
        DEBUG_PRINTLN(debug, ("[Error] Failed to start MIC I2S!"));
    } 
    if (wake_up_scheme == "edge_impulse")
    {
        wakeup_init(); 
 
        xTaskCreate(ESP_AI::wakeup_inference_wrapper, "wakeup_inference", 1024 * 6, this, 1, NULL);
 
    }


    if (String(server_config.ip) == "custom-made")
    {
        bool get_server_success = get_server_config();
        if (get_server_success == false)
        {
            DEBUG_PRINTLN(debug, ("[Error] 服务配置获取失败!"));
            return;
        }
    }
 

    xTaskCreate(
        ESP_AI::reporting_sensor_data_wrapper,
        "reporting_sensor_data",
        1024 * 4,
        this,
        1,
        NULL);
 

    xTaskCreate(
        ESP_AI::send_audio_wrapper,
        "send_audio",
        1024 * 2,
        this,
        1,
        NULL);
 

    xTaskCreate(
        ESP_AI::lights_wrapper,
        "lights",
        1024 * 2,
        this,
        1,
        NULL);

    xTaskCreate(
        ESP_AI::get_position_wrapper,
        "get_position",
        1024 * 4,
        this,
        1,
        NULL);

    xTaskCreate(
        ESP_AI::play_audio_wrapper,
        "play_audio",
        1024 * 4,
        this,
        1,
        NULL);
 

    if (volume_config.enable)
    {
        xTaskCreate(
            ESP_AI::volume_listener_wrapper,
            "volume_listener",
            1024 * 4,
            this,
            1,
            NULL);
    }
    connect_ws(); 
}
