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

#include "globals.h"
#include <vector>

bool esp_ai_start_ed = false;         // AST 服务推理中
bool esp_ai_ws_connected = false;     // ws 服务已经连接成功
String esp_ai_session_id = "";        // 会话ID，用于判断是否应该丢弃会话
String esp_ai_session_status = "";    // 会话状态
String esp_ai_tts_task_id = "";       // TTS chunk ID
String esp_ai_status = "";            // 设备状态
bool esp_ai_is_listen_model = true;   // 是否为按住对话模式, 这个模式下按住按钮才会进行聆听，放开后进行LLM推理
bool esp_ai_sleep = false;            // 是否为休眠状态，开发中...
bool esp_ai_played_connected = false; // 是否已经播放过 服务连接成功 的提示语了
bool asr_ing = false;                 // 硬件是否正在进行收音
int I2S_model = 3;                    // i2s 处于什么模式， 0 输入模式，1 输出模式, 3 闲置模式

// 开始将音频发往服务
bool esp_ai_start_send_audio = false;

#if !defined(LITTLE_ROM)
// 连接多个 wifi
WiFiMulti wifiMulti;
#endif
// 使用串口2进行串口通信
HardwareSerial Esp_ai_serial(2);
Preferences esi_ai_prefs;

// 网络状态
String esp_ai_net_status = "0";
// 是否是配网页面链接wifi时错处
String ap_connect_err = "0";

WebSocketsClient esp_ai_webSocket;
SemaphoreHandle_t esp_ai_ws_mutex;

bool spk_ing = false;
SemaphoreHandle_t audio_mutex = xSemaphoreCreateMutex();

MP3DecoderHelix esp_ai_dec_mp3;
#if defined(CODEC_TYPE_ES8311_NS4150)
DriverPins esp_ai_audio_pins;
AudioBoard esp_ai_audio_board(AudioDriverES8311, esp_ai_audio_pins);
AudioInfo esp_ai_audio_info(AUDIO_OUTPUT_SAMPLE_RATE, 1, 16);
I2SCodecStream esp_ai_spk_i2s(esp_ai_audio_board);
VolumeStream esp_ai_volume(esp_ai_spk_i2s);
BufferRTOS<uint8_t> esp_ai_audio_buffer(AUDIO_BUFFER_SIZE, AUDIO_CHUNK_SIZE);
QueueStream<uint8_t> esp_ai_spk_queue(esp_ai_audio_buffer);
// EncodedAudioStream esp_ai_dec(&esp_ai_spk_i2s, &esp_ai_dec_mp3);
EncodedAudioStream esp_ai_dec(&esp_ai_volume, &esp_ai_dec_mp3);
StreamCopy esp_ai_copier(esp_ai_dec, esp_ai_spk_queue);
BufferPrint esp_ai_spk_buffer_print(esp_ai_audio_buffer);

WebsocketStream ws_stream;
I2SCodecStream esp_ai_i2s_input(esp_ai_audio_board);
StreamCopy mic_to_ws_copier(ws_stream, esp_ai_i2s_input, AUDIO_COPY_CHUNK_SIZE);
#elif defined(CODEC_TYPE_ES8311_ES7210)
DriverPins esp_ai_audio_pins;
AudioBoard esp_ai_audio_board(AudioDriverES8311, esp_ai_audio_pins);
AudioInfo esp_ai_audio_info(AUDIO_OUTPUT_SAMPLE_RATE, 1, 16);
I2SCodecStream esp_ai_spk_i2s(esp_ai_audio_board);
VolumeStream esp_ai_volume(esp_ai_spk_i2s);
BufferRTOS<uint8_t> esp_ai_audio_buffer(AUDIO_BUFFER_SIZE, AUDIO_CHUNK_SIZE);
QueueStream<uint8_t> esp_ai_spk_queue(esp_ai_audio_buffer);
// EncodedAudioStream esp_ai_dec(&esp_ai_spk_i2s, &esp_ai_dec_mp3);
EncodedAudioStream esp_ai_dec(&esp_ai_volume, &esp_ai_dec_mp3);
StreamCopy esp_ai_copier(esp_ai_dec, esp_ai_spk_queue);
BufferPrint esp_ai_spk_buffer_print(esp_ai_audio_buffer);

WebsocketStream ws_stream;
DriverPins esp_ai_mic_pins;
AudioBoard esp_ai_mic_board(AudioDriverES7210, esp_ai_mic_pins);
AudioInfo esp_ai_mic_info(AUDIO_INPUT_SAMPLE_RATE, 1, 16);
I2SCodecStream esp_ai_i2s_input(esp_ai_mic_board);
StreamCopy mic_to_ws_copier(ws_stream, esp_ai_i2s_input, AUDIO_COPY_CHUNK_SIZE);

#else
I2SStream esp_ai_spk_i2s;
BufferRTOS<uint8_t> esp_ai_audio_buffer(AUDIO_BUFFER_SIZE, AUDIO_CHUNK_SIZE);
QueueStream<uint8_t> esp_ai_spk_queue(esp_ai_audio_buffer);
VolumeStream esp_ai_volume(esp_ai_spk_i2s);
EncodedAudioStream esp_ai_dec(&esp_ai_volume, &esp_ai_dec_mp3);
StreamCopy esp_ai_copier(esp_ai_dec, esp_ai_spk_queue);
BufferPrint esp_ai_spk_buffer_print(esp_ai_audio_buffer);

WebsocketStream ws_stream;
I2SStream esp_ai_i2s_input;
VolumeStream esp_ai_mic_volume(esp_ai_i2s_input);
StreamCopy mic_to_ws_copier(ws_stream, esp_ai_mic_volume, AUDIO_COPY_CHUNK_SIZE);
#endif

#if !defined(DISABLE_AP_NET)
WebServer esp_ai_server(80);
DNSServer esp_ai_dns_server;
#endif
#if defined(ARDUINO_XIAO_ESP32S3)
// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic default_i2s_config_mic = {I2S_PIN_NO_CHANGE, 42, 41};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {2, 3, 1, AUDIO_INPUT_SAMPLE_RATE};
// 重置按钮 { 输入引脚，电平： high | low}
ESP_AI_reset_btn_config default_reset_btn_config = {9, "high"};
// 灯光配置
ESP_AI_lights_config default_lights_config = {4, 1};
// 音量配置 { 输入引脚，输入最大值，默认音量 }
ESP_AI_volume_config default_volume_config = {7, 4096, 1, false};
// 默认离线唤醒方案
ESP_AI_wake_up_config default_wake_up_config = {"pin_high", 1, 10};
#elif defined(ARDUINO_ESP32C3_DEV)
ESP_AI_i2s_config_mic default_i2s_config_mic = {};
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {};
ESP_AI_reset_btn_config default_reset_btn_config = {BOOT_BUTTON_GPIO, "low"};
ESP_AI_lights_config default_lights_config = {BUILTIN_LED_GPIO, 1};
ESP_AI_volume_config default_volume_config = {};
ESP_AI_wake_up_config default_wake_up_config = {"pin_low", 1, BOOT_BUTTON_GPIO};
#else
// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num, i2s_bits_per_sample, 声道选择 }
ESP_AI_i2s_config_mic default_i2s_config_mic = {MIC_I2S_GPIO_BCLK, MIC_I2S_GPIO_WS, MIC_I2S_GPIO_DIN, I2S_BITS_PER_SAMPLE, I2S_CHANNEL_FMT_ONLY_LEFT};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {SPK_I2S_GPIO_BCLK, SPK_I2S_GPIO_WS, SPK_I2S_GPIO_DIN, AUDIO_INPUT_SAMPLE_RATE};
// 重置按钮 { 输入引脚，电平： high | low}
ESP_AI_reset_btn_config default_reset_btn_config = {RESET_BTN_GPIO, "high"};
// 灯光配置
ESP_AI_lights_config default_lights_config = {LIGHTS_GPIO, 1};
// 音量配置 { 输入引脚，输入最大值，默认音量 }
ESP_AI_volume_config default_volume_config = {VOL_GPIO, 4096, 1, false};
// 默认离线唤醒方案
ESP_AI_wake_up_config default_wake_up_config = {DEFAULT_WAKEUP_SCHEME, 1, 10};
#endif

// { wifi 账号， wifi 密码, "热点名字", "配网页面HTML", "配网方式：AP | BLE" }
#if defined(DISABLE_BLE_NET)
ESP_AI_wifi_config default_wifi_config = {"", "", "ESP-AI", "", "AP"};
#else
ESP_AI_wifi_config default_wifi_config = {"", "", "ESP-AI", "", "BLE"};
#endif

// { ip， port, api_key }
ESP_AI_server_config default_server_config = {"http", "node.espai.fun", 80, "", ""};
int mic_bits_per_sample = 16;

// 音频缓存
#if !defined(LITTLE_ROM)
std::vector<uint8_t> esp_ai_cache_audio_du;
std::vector<uint8_t> esp_ai_cache_audio_greetings;
#endif

String wake_up_scheme = WAKEUP_SCHEME; 
Adafruit_NeoPixel *esp_ai_pixels = nullptr;

// 获取ESP32的硬件地址（eFuse MAC地址）
String get_device_id()
{
    return WiFi.macAddress();
    // 正常应该用这个 MAC 地址，做一下倒叙即可。
    // uint64_t mac = ESP.getEfuseMac();
    // char macStr[18];
    // sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
    //         (uint8_t)(mac >> 40),
    //         (uint8_t)(mac >> 32),
    //         (uint8_t)(mac >> 24),
    //         (uint8_t)(mac >> 16),
    //         (uint8_t)(mac >> 8),
    //         (uint8_t)mac);
    // return macStr;
}

/**
 * 获取本地存储信息
 * get_local_data("wifi_name"); // oldwang
 */
String get_local_data(const String &field_name = "")
{
    if (field_name == "device_id")
    {
        return get_device_id();
    }
    esi_ai_prefs.begin("esp-ai-kv", false);
    String val = "";

    if (esi_ai_prefs.isKey(field_name.c_str()))
    {
        val = esi_ai_prefs.getString(field_name.c_str()).c_str();
    }
    esi_ai_prefs.end();
    return val;
}

JSONVar get_local_all_data()
{
    esi_ai_prefs.begin("esp-ai-kv", false);
    JSONVar data;

    String keys_list = esi_ai_prefs.getString("_keys_list_", "");

    if (keys_list != "")
    {
        int start = 0;
        int end = keys_list.indexOf(',');
        while (end != -1)
        {
            String key = keys_list.substring(start, end);
            String value = esi_ai_prefs.getString(key.c_str());
            data[key] = value;
            start = end + 1;
            end = keys_list.indexOf(',', start);
        }
        // 添加最后一个键值对
        String key = keys_list.substring(start);
        String value = esi_ai_prefs.getString(key.c_str());
        data[key] = value;
    }

    esi_ai_prefs.end();
    return data;
}

/**
 * 清除本地存储的所有信息
 */
void clear_local_all_data()
{
    // 擦除整个NVS分区（所有命名空间数据都会被清除）
    esp_err_t err = nvs_flash_erase();
    if (err != ESP_OK)
    {
        Serial.printf("NVS擦除失败: %d\n", err);
    }
    else
    {
        Serial.println("NVS擦除成功");
        // 擦除后需要重新初始化NVS
        err = nvs_flash_init();
        if (err != ESP_OK)
        {
            Serial.printf("NVS初始化失败: %d\n", err);
        }
    }
}

/**
 * set_local_data("wifi_name", "oldwang");
 */
void set_local_data(String field_name, String new_value)
{
    esi_ai_prefs.begin("esp-ai-kv", false);
    String keys_list = esi_ai_prefs.getString("_keys_list_", "");
    bool key_exists = false;
    if (keys_list != "")
    {
        // 将键列表拆分为数组进行精确匹配
        int index = 0;
        while (index < keys_list.length())
        {
            int next_comma = keys_list.indexOf(',', index);
            String key;
            if (next_comma == -1)
            {
                key = keys_list.substring(index);
                index = keys_list.length();
            }
            else
            {
                key = keys_list.substring(index, next_comma);
                index = next_comma + 1;
            }

            if (key == field_name)
            {
                key_exists = true;
                break;
            }
        }
    }

    // 如果键不存在，则添加到键列表
    if (!key_exists)
    {
        if (keys_list != "")
        {
            keys_list += ",";
        }
        keys_list += field_name;
        esi_ai_prefs.putString("_keys_list_", keys_list);
    }

    // 存储或更新键值对
    esi_ai_prefs.putString(field_name.c_str(), new_value.c_str());
    esi_ai_prefs.end();
}

/**
 * 将角度转换为占空比
 */
int angleToDutyCycle(int angle)
{
    // 舵机的脉冲宽度范围通常在 0.5ms - 2.5ms
    // 对于 50Hz 的频率，周期为 20ms
    // 分辨率为 10 位时，最大值为 1023
    int dutyCycle = map(angle, 0, 180, 26, 123);
    return dutyCycle;
}

std::vector<int> digital_read_pins;
std::vector<int> analog_read_pins;

String decodeURIComponent(const String &encoded)
{
    String decoded = "";
    for (int i = 0; i < encoded.length(); i++)
    {
        if (encoded[i] == '%')
        {
            if (i + 2 < encoded.length())
            {
                // 提取 % 后面的两位十六进制字符
                String hexStr = encoded.substring(i + 1, i + 3);
                char decodedChar = (char)strtol(hexStr.c_str(), NULL, 16);
                decoded += decodedChar;
                i += 2; // 跳过已经处理的两位十六进制字符
            }
            else
            {
                // 不完整的 % 序列，按原样添加
                decoded += encoded[i];
            }
        }
        else if (encoded[i] == '+')
        {
            // 处理 + 符号，它通常代表空格
            decoded += ' ';
        }
        else
        {
            // 普通字符直接添加到结果中
            decoded += encoded[i];
        }
    }
    return decoded;
}

String get_ap_name(String ap_name)
{
    String device_id = get_local_data("device_id");
    String lastTwoChars = device_id.substring(device_id.length() - 5);
    lastTwoChars.replace(":", "");
    String name = ap_name.length() > 0 ? ap_name : "ESP-AI:" + lastTwoChars;
    return name;
}

// 接收到的客户端蓝牙数据
String ESP_AI_BLE_RD = "";
String ESP_AI_BLE_ERR = "";

void espai_system_mem_init()
{
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
        Serial.println(F("[Info] PSRAM 无效，如果您使用的是 esp32s3 开发板，请【设置/PSRAM/OPI PSRAM】，否则请忽略！"));
        Serial.println(F("[Info] 注意分区方案需要选择： 16MB Flash(3MB APP/9.9MB FATFS)"));
    }
}

void print_task_info(void)
{
    // 获取当前任务的句柄
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();
    // 获取当前任务的名字
    const char *taskName = pcTaskGetName(currentTaskHandle);
    // 获取当前任务的堆栈高水位标记
    UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(currentTaskHandle);
    Serial.printf("任务名称: %s 剩余堆栈: %d bytes\n", taskName, stackHighWaterMark);
}

bool mp3_player_is_playing()
{
    return esp_ai_spk_queue.available();
}

void mp3_player_write(const unsigned char *data, size_t len)
{
    if (len > 0)
    {
        if (esp_ai_volume.volume(0) == 0)
        {
            esp_ai_volume.setVolume(1); // 设置音量为 70%
        }

        size_t remaining = len;
        const unsigned char *current = data;
        while (remaining > 0)
        {
            size_t chunk_size = (remaining < AUDIO_CHUNK_SIZE) ? remaining : AUDIO_CHUNK_SIZE;
            esp_ai_spk_buffer_print.write(current, chunk_size);
            current += chunk_size;
            remaining -= chunk_size;
        }
    }
}

void mp3_player_stop()
{
    // 必须先拿到spk锁, 避免  _buffer.writeArray 写入卡死导致解锁失败。
    xSemaphoreTake(audio_mutex, portMAX_DELAY);
    spk_ing = false;
    xSemaphoreGive(audio_mutex); 

    // 进入临界区
    xSemaphoreTake(audio_mutex, portMAX_DELAY);
    esp_ai_spk_buffer_print.reset();
    esp_ai_audio_buffer.reset();
    esp_ai_spk_queue.flush();
    esp_ai_spk_i2s.flush(); 

    xSemaphoreGive(audio_mutex);
    // esp_ai_volume.setVolume(0);
    wait_mp3_player_done(); // 停止后必须把剩余的播放完毕
}

void wait_mp3_player_done()
{
    while (mp3_player_is_playing())
    {
        vTaskDelay(pdMS_TO_TICKS(30));
        // Serial.printf("*");
    }
    // Serial.printf("\n");

    vTaskDelay(pdMS_TO_TICKS(300));
}

void play_builtin_audio(const unsigned char *data, size_t len)
{
    open_spk();
    if (!spk_ing)
    {
        // 必须先拿到spk锁, 避免  _buffer.writeArray 写入卡死导致解锁失败。
        xSemaphoreTake(audio_mutex, portMAX_DELAY);
        spk_ing = true;
        xSemaphoreGive(audio_mutex);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    mp3_player_write(data, len); 
    wait_mp3_player_done();
    spk_ing = false;
}

void open_spk()
{
#if defined(CODEC_TYPE_ES8311_NS4150)
    if (I2S_model == 1)
        return;
    // 切换播放模式
    esp_ai_i2s_input.flush();
    esp_ai_i2s_input.end();

    digitalWrite(AUDIO_CODEC_PA_PIN, HIGH);
    static auto config = esp_ai_spk_i2s.defaultConfig(TX_MODE);
    config.copyFrom(esp_ai_audio_info); 
    esp_ai_spk_i2s.begin(config);
    vTaskDelay(pdMS_TO_TICKS(50));
    I2S_model = 1;
#endif
}

void open_mic()
{
#if defined(CODEC_TYPE_ES8311_NS4150)

    esp_ai_spk_i2s.flush();          // 停止 I2S 输出
    esp_ai_spk_queue.flush();        // 清空 QueueStream
    esp_ai_audio_buffer.reset();     // 重置 ring buffer
    esp_ai_spk_buffer_print.reset(); // 重置输出 buffer

    if (I2S_model == 0)
        return;

    esp_ai_spk_i2s.end();

    digitalWrite(AUDIO_CODEC_PA_PIN, LOW);
    static auto config_mic = esp_ai_i2s_input.defaultConfig(RX_MODE);
    config_mic.copyFrom(esp_ai_audio_info);
    config_mic.sample_rate = 16000;
    config_mic.channels = 1;
    esp_ai_i2s_input.begin(config_mic);
    mic_to_ws_copier.begin(ws_stream, esp_ai_i2s_input);
    vTaskDelay(pdMS_TO_TICKS(50));
    I2S_model = 0;
#endif
}
 

NimBLEServer *esp_ai_ble_server;
NimBLECharacteristic *esp_ai_ble_characteristic;
NimBLEService *esp_ai_ble_service;
NimBLEAdvertising *esp_ai_ble_advertising;
