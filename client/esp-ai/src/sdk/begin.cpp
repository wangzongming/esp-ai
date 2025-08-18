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
#include "../private/lights.h"
#include "../private/play_audio.h"
#include "../private/on_repeatedly_click.h"
#include "../private/on_wakeup.h"
#include "../private/get_position.h"
#include "../private/volume_listener.h"

LightContext light_ctx;
PlayAudioContext play_audio_ctx;
OnRepeatedlyClickContext on_repeatedly_click_ctx;
OnWakeUpContext on_wake_up_ctx;
GetPositionContext get_position_ctx;
VolListenContext vol_listen_ctx;

void ESP_AI::begin(ESP_AI_CONFIG config)
{

#if defined(ARDUINO_XIAO_ESP32S3)
    Serial.println(F("[Info] 检测到 XIAO ESP32S3 开发板"));
    delay(500);
#elif defined(ARDUINO_ESP32S3_DEV)
    Serial.println(F("[Info] 检测到 ESP32-S3 开发板"));
#elif defined(ARDUINO_ESP32C3_DEV)
    Serial.println(F("[Info] 检测到 ESP32-C3 开发板"));
    delay(1000);
#else
    Serial.println(F("[Error] 您的开发板可能不受支持！"));
#endif

    esp_ai_ws_mutex = xSemaphoreCreateMutex();

    if (strcmp(config.wake_up_config.wake_up_scheme, "asrpro") == 0 || strcmp(config.wake_up_config.wake_up_scheme, "serial") == 0)
    {
#if !defined(IS_BOWKNOT)
        // 参数包括串行通信的波特率、串行模式、使用的 RX 引脚和 TX 引脚。
        Esp_ai_serial.begin(115200, SERIAL_8N1, esp_ai_serial_rx, esp_ai_serial_tx);
#endif
    }

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
#if !defined(DISABLE_BLE_NET)
        wifi_config.way = "BLE"; // 默认使用热点配网
#else
        wifi_config.way = "AP"; // 默认使用热点配网
#endif
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

    int pin = 18;
    int count = 1;
#if defined(ARDUINO_ESP32C3_DEV)
    pin = BUILTIN_LED_GPIO;
#endif
#if defined(ARDUINO_XIAO_ESP32S3)
    pin = 4;
#endif

    if (config.lights_config.pin)
    {
        // lights_config = config.lights_config;
        lights_config.pin = config.lights_config.pin;
        pin = lights_config.pin;
    }
    if (config.lights_config.count)
    {
        count = config.lights_config.count;
        lights_config.count = config.lights_config.count;
    }

    esp_ai_pixels = new Adafruit_NeoPixel((uint16_t)count, (uint16_t)pin, NEO_GRB + NEO_KHZ800);
    esp_ai_pixels->begin();
    esp_ai_pixels->setBrightness(100); // 亮度设置
    esp_ai_pixels->clear();            // 将所有像素颜色设置为“off”
    esp_ai_pixels->show();             // Initialize all pixels to 'off'

    // 灯光任务比较重要，靠前执行
    light_ctx = {
        .pixels = esp_ai_pixels,
        .count = lights_config.count,
        .status = &esp_ai_status,
        .esp_ai_start_send_audio = &esp_ai_start_send_audio,
        .isPlaying = mp3_player_is_playing,
    };
    xTaskCreateStatic(light_task_static, "lights", LIGHTS_TASK_SIZE, &light_ctx, 1, lightsTaskStack, &lightsTaskBuffer);

    // 初始化扬声器
    AudioLogger::instance().begin(Serial, AudioLogger::Error);
    // AudioLogger::instance().begin(Serial, AudioLogger::Info);
    // AudioLogger::instance().begin(Serial, AudioLogger::Debug);
#if defined(CODEC_TYPE_BLAMP_I2S)
    auto esp_ai_spk_config = esp_ai_spk_i2s.defaultConfig(TX_MODE);
    esp_ai_spk_config.sample_rate = i2s_config_speaker.sample_rate ? i2s_config_speaker.sample_rate : 16000;
    esp_ai_spk_config.bits_per_sample = 16;
    esp_ai_spk_config.port_no = YSQ_i2s_num;
    esp_ai_spk_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;

    esp_ai_spk_config.i2s_format = I2S_MSB_FORMAT;
    esp_ai_spk_config.buffer_count = 8;
    esp_ai_spk_config.buffer_size = 1024;
    esp_ai_spk_config.auto_clear = true;
    esp_ai_spk_config.channels = 1;
    esp_ai_spk_config.pin_ws = i2s_config_speaker.ws_io_num;     // LCK
    esp_ai_spk_config.pin_bck = i2s_config_speaker.bck_io_num;   // BCK
    esp_ai_spk_config.pin_data = i2s_config_speaker.data_in_num; // DIN

    esp_ai_spk_i2s.begin(esp_ai_spk_config);
    esp_ai_spk_queue.begin();
    esp_ai_volume.begin(esp_ai_spk_config);
    setVolume(volume_config.volume);
    esp_ai_dec.begin(esp_ai_spk_config);
    esp_ai_copier.begin();
    esp_ai_dec_mp3.setMaxFrameSize(32);
#endif

#if defined(CODEC_TYPE_ES8311_ES7210)
    pinMode(AUDIO_CODEC_PA_PIN, OUTPUT);
    digitalWrite(AUDIO_CODEC_PA_PIN, HIGH); // 打开功放
    Serial.println("[Info] audio board begin...");
    Serial.printf("AUDIO_CODEC_I2C_SCL_PIN=%d, AUDIO_CODEC_I2C_SDA_PIN=%d\n", AUDIO_CODEC_I2C_SCL_PIN, AUDIO_CODEC_I2C_SDA_PIN);
    esp_ai_audio_pins.addI2C(PinFunction::CODEC, AUDIO_CODEC_I2C_SCL_PIN, AUDIO_CODEC_I2C_SDA_PIN, AUDIO_CODEC_ES8311_ADDR, AUDIO_CODEC_I2C_SPEED);
    esp_ai_audio_pins.addI2S(PinFunction::CODEC, SPK_I2S_GPIO_MCLK, SPK_I2S_GPIO_BCLK, SPK_I2S_GPIO_WS, SPK_I2S_GPIO_DOUT, SPK_I2S_GPIO_DIN);
    esp_ai_audio_pins.begin();
    esp_ai_audio_board.begin();
    Serial.println("[Info] mic board begin...");
    esp_ai_mic_pins.addI2C(PinFunction::CODEC, AUDIO_CODEC_I2C_SCL_PIN, AUDIO_CODEC_I2C_SDA_PIN, AUDIO_CODEC_ES7210_ADDR, AUDIO_CODEC_I2C_SPEED);
    esp_ai_mic_pins.addI2S(PinFunction::CODEC, MIC_I2S_GPIO_MCLK, MIC_I2S_GPIO_BCLK, MIC_I2S_GPIO_WS, MIC_I2S_GPIO_DOUT, MIC_I2S_GPIO_DIN);
    esp_ai_mic_pins.begin();
    esp_ai_mic_board.begin();

    Serial.println("[Info] esp_ai_i2s_input begin...");
    // === 配置 I2S + ES8311 ===
    auto config_spk = esp_ai_spk_i2s.defaultConfig(TX_MODE);
    config_spk.copyFrom(esp_ai_audio_info);
    config_spk.sample_rate = 16000;
    config_spk.channels = 1;
    esp_ai_spk_queue.begin();
    esp_ai_dec.begin(config_spk);
    esp_ai_volume.begin(config_spk);
    setVolume(1);
    esp_ai_spk_i2s.begin(config_spk);
    Serial.println("[Info] esp_ai_i2s_input begin...");
    // // === 配置 I2S + ES7210 ===
    auto config_mic = esp_ai_i2s_input.defaultConfig(RX_MODE);
    config_mic.copyFrom(esp_ai_mic_info);
    config_mic.sample_rate = 16000;
    config_mic.port_no = MIC_i2s_num;
    config_mic.channels = 1;
    esp_ai_i2s_input.begin(config_mic);
    mic_to_ws_copier.begin(ws_stream, esp_ai_i2s_input);
#endif
    play_audio_ctx.available = []()
    { return esp_ai_spk_queue.available(); };
    play_audio_ctx.copy = []()
    {
        esp_ai_copier.copy();
    };
    play_audio_ctx.sendTXT = [](const char *msg)
    {
        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            esp_ai_webSocket.sendTXT(msg);
            xSemaphoreGive(esp_ai_ws_mutex);
        }
    };
    play_audio_ctx.spk_ing = &spk_ing;
    play_audio_ctx.esp_ai_ws_connected = &esp_ai_ws_connected;
    play_audio_ctx.esp_ai_session_id = &esp_ai_session_id;
    xTaskCreateStatic(play_audio_task_static, "play_audio", PLAY_AUDIO_TASK_SIZE, &play_audio_ctx, 1, playAudioTaskStack, &playAudioTaskBuffer);

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
            return;
    }

    bool not_wifi_info = true;
#if defined(LITTLE_ROM)
    String loc_wifi_name[1] = {""};
    String loc_wifi_pwd[1] = {""};
    loc_wifi_name[0] = get_local_data("wifi_name");
    loc_wifi_pwd[0] = get_local_data("wifi_pwd");
    if (loc_wifi_name[0] != "")
    {
        not_wifi_info = false;
    }
#else
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

    for (int i = 0; i < 5; i++)
    {
        if (loc_wifi_name[i] != "") // ssid 不能为空
        {
            not_wifi_info = false;
        }
    }
#endif

    if (not_wifi_info)
    {
        loc_wifi_name[0] = wifi_config.wifi_name;
        loc_wifi_pwd[0] = wifi_config.wifi_pwd;
    }

    DEBUG_PRINTLN(debug, F("==================== Connect WIFI ===================="));
    ap_connect_err = "0";

    // wifi_pwd 可以为空（无密码）
    if (loc_wifi_name[0] == "" && not_wifi_info)
    {
        DEBUG_PRINTLN(debug, F("[Info] 没有wifi信息，请配网"));
        DEBUG_PRINT(debug, F("[Info] 配网方式："));
        DEBUG_PRINTLN(debug, wifi_config.way);
        if (wifi_config.way == "BLE")
        {
#if !defined(DISABLE_BLE_NET)
            open_ble_server();
#endif
        }
        else
        {
#if !defined(DISABLE_AP_NET)
            open_ap();
#endif
        }
        return;
    }

    on_repeatedly_click_ctx = {
        .debug = &debug,
        .esp_ai_session_id = &esp_ai_session_id,
        .power = &reset_btn_config.power,
        .pin = &reset_btn_config.pin,
    };
    on_repeatedly_click_ctx.play_builtin_audio = play_builtin_audio;
    on_repeatedly_click_ctx.wait_mp3_player_done = wait_mp3_player_done;
    on_repeatedly_click_ctx.on_repeatedly_click_cb = [this]()
    {
        if (this->onRepeatedlyClickCb != nullptr)
            this->onRepeatedlyClickCb();
    };
    on_repeatedly_click_ctx.clear_data = [this]()
    {
        this->clearData();
    };
    xTaskCreateStatic(on_repeatedly_click_task_static, "on_repeatedly_click", ON_REPEATEDLY_CLICK_TASK_SIZE, &on_repeatedly_click_ctx, 1, onRepeatedlyClickTaskStack, &onRepeatedlyClickTaskBuffer);

// 处理蓝牙临时数据
#if !defined(DISABLE_BLE_NET)
    String loc_ble_temp_ = get_local_data("_ble_temp_");
    if (loc_ble_temp_ == "1")
    {
        ble_connect_wifi();
        return;
    }
#endif

    play_builtin_audio(lian_jie_zhong, lian_jie_zhong_len);
    awaitPlayerDone();

    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
#if defined(LITTLE_ROM)
    WiFi.begin(loc_wifi_name[0], loc_wifi_pwd[0]);
    int connect_count = 0;
    // 10s 连不上Wifi的话
    int try_count = 15;
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
        delay(250);
        DEBUG_PRINT(debug, "."); // 设备状态回调
        // 内置状态处理
        status_change("0_ing_after");
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing_after");
        }
        delay(250);
    }

#else
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
    }

#endif

    if (WiFi.status() != WL_CONNECTED)
    {
        if (wifi_config.way == "BLE")
        {
#if !defined(DISABLE_BLE_NET)
            open_ble_server();
#endif
        }
        else
        {

#if !defined(DISABLE_AP_NET)
            open_ap();
#endif
        }
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);
        return;
    }


    esp_ai_played_connected = false;
    // 内置状态处理
    status_change("2");
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "2";
        onNetStatusCb("2");
    }
    
    play_builtin_audio(lian_jie_cheng_gong, lian_jie_cheng_gong_len);
    awaitPlayerDone();
    play_builtin_audio(fu_wu_lian_jie_zhong, fu_wu_lian_jie_zhong_len);

    WiFi.setSleep(false);

    DEBUG_PRINTLN(debug, "");
    DEBUG_PRINT(debug, F("[Info] wifi 连接成功。"));
    String ip_str = WiFi.localIP().toString();
    if (onConnectedWifiCb != nullptr)
    {
        onConnectedWifiCb(ip_str);
    }

#if defined(CODEC_TYPE_BLAMP_I2S)
    if (mic_i2s_init(16000))
    {
        DEBUG_PRINTLN(debug, F("[Error] Failed to start MIC I2S!"));
    }

    if (wake_up_scheme == "edge_impulse")
    {
        Serial.println(F("[Error] edge_impulseh唤醒方案已经废弃，请尝试其他唤醒方案。"));
        return;
    }
#endif

#if !defined(LITTLE_ROM)

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
#endif

    on_wake_up_ctx.debug = &debug;
    on_wake_up_ctx.asr_ing = &asr_ing;
    on_wake_up_ctx.pin = &wake_up_config.pin;
    on_wake_up_ctx.wake_up_scheme = &wake_up_scheme;
    on_wake_up_ctx.esp_ai_is_listen_model = &esp_ai_is_listen_model;
    on_wake_up_ctx.Esp_ai_serial = &Esp_ai_serial;
    on_wake_up_ctx.esp_ai_start_send_audio = &esp_ai_start_send_audio;
    on_wake_up_ctx.wake_up_str = wake_up_config.str;

    // lambda 捕获 this 时必须分开赋值，不能放进 {} 初始化器中
    on_wake_up_ctx.wakeUp = [this](const char *msg)
    {
        this->wakeUp(msg);
    };

    on_wake_up_ctx.sendTXT = [](const char *msg)
    {
        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            esp_ai_webSocket.sendTXT(msg);
            xSemaphoreGive(esp_ai_ws_mutex);
        }
    };
    xTaskCreateStatic(on_wakeup_task_static, "on_wakeup", ON_WAKE_UP_TASK_SIZE, &on_wake_up_ctx, 1, onWakeupTaskStack, &onWakeupTaskBuffer);

    get_position_ctx.debug = &debug;
    get_position_ctx.IS_WL_CONNECTED = []()
    { return WiFi.status() == WL_CONNECTED; };
    get_position_ctx.onPositionCb = [this](const String &ip, const String &nation, const String &province, const String &city, const String &latitude, const String &longitude)
    {
        if (this->onPositionCb != nullptr)
        {
            this->onPositionCb(ip, nation, province, city, latitude, longitude);
        }
    };
    xTaskCreateStatic(get_position_task_static, "get_position", GET_POSITION_TASK_SIZE, &get_position_ctx, 1, getPositionContextTaskStack, &getPositionContextTaskBuffer);

    if (volume_config.enable)
    {

        vol_listen_ctx.pin = &volume_config.input_pin;
        vol_listen_ctx.max_val = &volume_config.max_val;
        vol_listen_ctx.onChange = [this](float formattedNumber)
        {
            if (fabs(this->volume_config.volume - formattedNumber) >= 0.1)
            {
                setVolume(formattedNumber);
            }
        };
        xTaskCreateStatic(vol_listen_task_static, "volume_listener", VOL_LISTEN_TASK_SIZE, &vol_listen_ctx, 1, volListenTaskStack, &volListenTaskBuffer);
    }

    connect_ws();
}
