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

#include "begin.h"

void ESP_AI::begin(ESP_AI_CONFIG config)
{
    // xiao 需要延迟一定的时间
    delay(500);

#if defined(ARDUINO_XIAO_ESP32S3)
    Serial.println(F("[Info] 检测到 XIAO ESP32S3 开发板"));
#elif defined(ARDUINO_ESP32S3_DEV)
    Serial.println(F("[Info] 检测到 ESP32-S3 开发板"));
#else
    Serial.println(F("[Error] 您的开发板可能不受支持！"));
#endif

    esp_ai_asr_sample_buffer_before = new std::vector<uint8_t>();
    esp_ai_asr_sample_buffer_before->reserve(48000 * sizeof(int16_t));

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
        Serial.println(F("[Error] PSRAM 无效，请确保您使用的是 esp32s3 开发板，并且开启了【设置/PSRAM/OPI PSRAM】"));
        Serial.println(F("[Error] 注意分区方案需要选择： 16MB Flash(3MB APP/9.9MB FATFS)"));
    }

    if (config.i2s_config_mic.bck_io_num)
    {
        i2s_config_mic = config.i2s_config_mic;
    }
    if (config.i2s_config_speaker.bck_io_num)
    {
        i2s_config_speaker = config.i2s_config_speaker;
    }

    // wifi 配置
    if (strcmp(config.wifi_config.wifi_name, "") != 0 || strcmp(config.wifi_config.ap_name, "") != 0 || config.wifi_config.html_str != "" || config.wifi_config.way != "")
    {
        wifi_config = config.wifi_config;
    }
    if (wifi_config.way == "")
    {
        wifi_config.way = "AP"; // 默认使用热点配网
    }

    // 服务配置
    if (strcmp(config.server_config.ip, "") != 0)
    {
        server_config = config.server_config;
    }

    // 音量控制配置
    if (config.volume_config.input_pin)
    {
        volume_config = config.volume_config;
    }
    if (config.debug)
    {
        debug = config.debug;
    }

    // 重置按钮配置
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

    // 唤醒配置
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
            wake_up_config.vad_first = 10000;
        }
        if (!wake_up_config.vad_course)
        {
            wake_up_config.vad_course = 500;
        }
    }
    esp_ai_is_listen_model = (wake_up_scheme == "pin_high_listen" || wake_up_scheme == "pin_low_listen");

    // 将按钮默认拉低
    if (wake_up_scheme != "pin_high" && wake_up_scheme != "pin_low" && config.reset_btn_config.power != "high" && config.reset_btn_config.power != "low")
    {
        pinMode(wake_up_config.pin, INPUT_PULLDOWN);
    }

    // 按住对话方式配置
    if (config.lights_config.pin)
    {
        lights_config = config.lights_config;
        esp_ai_pixels.setPin(lights_config.pin);
    } 

    // ws2812
    esp_ai_pixels.begin();
    esp_ai_pixels.setBrightness(100); // 亮度设置
    esp_ai_pixels.clear();            // 将所有像素颜色设置为“off”
    esp_ai_pixels.show();             // Initialize all pixels to 'off'

    // 灯光任务比较重要，靠前执行
    xTaskCreate(ESP_AI::lights_wrapper, "lights", 1024 * 2, this, 1, NULL);

    // 初始化扬声器
    speaker_i2s_setup();

    // 内置状态处理
    status_change("0");
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "0";
        onNetStatusCb("0");
    }

    // 在开始前让留给开发者决策是否能继续执行
    if (onBeginCb != nullptr)
    {
        bool can_bagin = onBeginCb();
        if (can_bagin == false)
        {
            return;
        }
    }

    String loc_device_id = get_device_id();
    String loc_wifi_name = "";
    String loc_wifi_pwd = "";
    JSONVar data = get_local_all_data();
    JSONVar keys = data.keys();
    for (int i = 0; i < keys.length(); i++)
    {
        String key = keys[i];
        if (key != "")
        {
            if (key == "wifi_name")
            {
                loc_wifi_name = (const char *)data[key];
            }
            if (key == "wifi_pwd")
            {
                loc_wifi_pwd = (const char *)data[key];
            }

            DEBUG_PRINT(debug, "[Info] 本地数据 " + key + " :");
            DEBUG_PRINTLN(debug, (const char *)data[key]);
        }
    }

    xTaskCreate(ESP_AI::on_repeatedly_click_wrapper, "on_repeatedly_click", 1024 * 4, this, 1, NULL);

    DEBUG_PRINTLN(debug, F("==================== Connect WIFI ===================="));
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
        DEBUG_PRINTLN(debug, F("[Info] 没有wifi信息，请配网"));
        DEBUG_PRINT(debug, F("[Info] 配网方式：" ));
        DEBUG_PRINTLN(debug, wifi_config.way);
        if (wifi_config.way == "BLE")
        {
            open_ble_server();
        }
        else
        {
            open_ap();
        }
        return;
    }

    // 处理蓝牙临时数据
    String loc__ble_temp_ = get_local_data("_ble_temp_"); 
    if(loc__ble_temp_ == "1"){
        ble_connect_wifi();
        return;
    }

    esp_ai_dec.write(lian_jie_zhong, lian_jie_zhong_len);

    DEBUG_PRINTLN(debug, "[Info] 连接 WIFI: " + _wifi_name);
    DEBUG_PRINTLN(debug, "[Info] WIFI 密码: " + _wifi_pwd);

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
            DEBUG_PRINTLN(debug, F("\n[Error] 连接WIFI失败，请重新配网"));
            if (wifi_config.way == "BLE")
            {
                open_ble_server();
            }
            else
            {
                open_ap();
            }
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

    DEBUG_PRINTLN(debug, F("==============================================="));

    if (mic_i2s_init(16000))
    {
        DEBUG_PRINTLN(debug, F("[Error] Failed to start MIC I2S!"));
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
            DEBUG_PRINTLN(debug, F("[Error] 服务配置获取失败!"));
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
        ESP_AI::on_wakeup_wrapper,
        "on_wakeup",
        1024 * 4,
        this,
        1, NULL);

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
