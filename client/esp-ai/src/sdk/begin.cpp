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

    if (debug)
    {
        esp_log_level_set("wifi", ESP_LOG_VERBOSE); // WiFi详细日志
    }

    esp_ai_ws_mutex = xSemaphoreCreateMutex();

#if defined(ARDUINO_XIAO_ESP32S3)
    Serial.println(F("[Info] 检测到 XIAO ESP32S3 开发板"));
#elif defined(ARDUINO_ESP32S3_DEV)
    Serial.println(F("[Info] 检测到 ESP32-S3 开发板"));
#else
    Serial.println(F("[Error] 您的开发板可能不受支持！"));
#endif

    // 参数包括串行通信的波特率、串行模式、使用的 RX 引脚和 TX 引脚。
    Esp_ai_serial.begin(115200, SERIAL_8N1, esp_ai_serial_rx, esp_ai_serial_tx);

    // 内存初始化
    espai_system_mem_init();

    if (config.i2s_config_mic.bck_io_num)
    {
        i2s_config_mic = config.i2s_config_mic;
    }
    if (config.i2s_config_speaker.bck_io_num)
    {
        i2s_config_speaker = config.i2s_config_speaker;
    }


    if (config.i2s_config_mic.bits_per_sample)
    {
        mic_bits_per_sample = config.i2s_config_mic.bits_per_sample;
    }
    else
    {
        mic_bits_per_sample = 16; 
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
    xTaskCreate(ESP_AI::lights_wrapper, "lights", 1024 * 3, this, 1, NULL);

    // 初始化扬声器
    speaker_i2s_setup();
    xTaskCreate(ESP_AI::play_audio_wrapper, "play_audio", 1024 * 4, this, 1, NULL);

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
    // 依次读取 5 组 wifi 信息
    String loc_wifi_name[5] = {"", "", "", "", ""};
    String loc_wifi_pwd[5] = {"", "", "", "", ""};
    JSONVar data = get_local_all_data();
    JSONVar keys = data.keys();
    for (int i = 0; i < keys.length(); i++)
    {
        String key = keys[i];
        if (key != "")
        {
            if (key == "wifi_name")
            {
                loc_wifi_name[0] = (const char *)data[key];
            }
            if (key == "wifi_name2")
            {
                loc_wifi_name[1] = (const char *)data[key];
            }
            if (key == "wifi_name3")
            {
                loc_wifi_name[2] = (const char *)data[key];
            }
            if (key == "wifi_name4")
            {
                loc_wifi_name[3] = (const char *)data[key];
            }
            if (key == "wifi_name5")
            {
                loc_wifi_name[4] = (const char *)data[key];
            }
            if (key == "wifi_pwd")
            {
                loc_wifi_pwd[0] = (const char *)data[key];
            }
            if (key == "wifi_pwd2")
            {
                loc_wifi_pwd[1] = (const char *)data[key];
            }
            if (key == "wifi_pwd3")
            {
                loc_wifi_pwd[2] = (const char *)data[key];
            }
            if (key == "wifi_pwd4")
            {
                loc_wifi_pwd[3] = (const char *)data[key];
            }
            if (key == "wifi_pwd5")
            {
                loc_wifi_pwd[4] = (const char *)data[key];
            }

            DEBUG_PRINT(debug, "[Info] 本地数据 " + key + " :");
            DEBUG_PRINTLN(debug, (const char *)data[key]);
        }
    }

    xTaskCreate(ESP_AI::on_repeatedly_click_wrapper, "on_repeatedly_click", 1024 * 4, this, 1, NULL);

    DEBUG_PRINTLN(debug, F("==================== Connect WIFI ===================="));
    ap_connect_err = "0";
    if (loc_wifi_name[0] == "")
    {
        loc_wifi_name[0] = wifi_config.wifi_name;
        loc_wifi_pwd[0] = wifi_config.wifi_pwd;
    }
    // wifi_pwd 可以为空（无密码）

    if (loc_wifi_name[0] == "")
    {
        DEBUG_PRINTLN(debug, F("[Info] 没有wifi信息，请配网"));
        DEBUG_PRINT(debug, F("[Info] 配网方式："));
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
    if (loc__ble_temp_ == "1")
    {
        ble_connect_wifi();
        return;
    }

    play_builtin_audio(lian_jie_zhong, lian_jie_zhong_len);

    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    for (int i = 0; i < 5; i++)
    {
        if (loc_wifi_name[i] != "") // ssid 不能为空
        {
            DEBUG_PRINTLN(debug, "[Info] 连接 WIFI" + String(i + 1) + ": " + loc_wifi_name[i]);
            DEBUG_PRINTLN(debug, "[Info] WIFI" + String(i + 1) + " 密码: " + loc_wifi_pwd[i]);
            wifiMulti.addAP(loc_wifi_name[i].c_str(), loc_wifi_pwd[i].c_str());
        }
    }

    DEBUG_PRINT(debug, F("[Info] connect wifi ing.."));

    // 等待 5 秒
    if (wifiMulti.run(5000) != WL_CONNECTED)
    {
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

    if (WiFi.status() != WL_CONNECTED)
    {
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);
        return;
    }

    play_builtin_audio(lian_jie_cheng_gong, lian_jie_cheng_gong_len);
    play_builtin_audio(fu_wu_lian_jie_zhong, fu_wu_lian_jie_zhong_len);

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

    if (mic_i2s_init(16000))
    {
        DEBUG_PRINTLN(debug, F("[Error] Failed to start MIC I2S!"));
    }

    if (wake_up_scheme == "edge_impulse")
    {
        wakeup_init();

        xTaskCreate(ESP_AI::wakeup_inference_wrapper, "wakeup_inference", 1024 * 6, this, 1, &wakeup_task_handle);
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
        1024,
        this,
        1,
        &sensor_task_handle);

    xTaskCreate(
        ESP_AI::on_wakeup_wrapper,
        "on_wakeup",
        1024 * 4,
        this,
        1, &on_wakeup_task_handle);

    xTaskCreate(
        ESP_AI::get_position_wrapper,
        "get_position",
        1024 * 4,
        this,
        1,
        &get_position_task_handle);

    xTaskCreate(
        ESP_AI::send_audio_wrapper,
        "send_audio",
        1024 * 8,
        this,
        1,
        &send_audio_task_handle);

    if (volume_config.enable)
    {
        xTaskCreate(
            ESP_AI::volume_listener_wrapper,
            "volume_listener",
            1024 * 4,
            this,
            1,
            &volume_listener_task_handle);
    }
    connect_ws();
}
