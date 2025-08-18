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

#pragma once
#include "globals.h"

class ESP_AI
{
public:
    ESP_AI();
    void begin(ESP_AI_CONFIG config);
    void loop();
    /**
     * 返回是否连接WiFi
     */
    bool wifiIsConnected() { return WiFi.status() == WL_CONNECTED; };
    /**
     * 返回本地IP
     * 调用案例： Serial.println(ESP_AI.localIP().c_str());
     */
    std::string localIP() { return std::string(WiFi.localIP().toString().c_str()); };
    void wakeUp(const String &scene = "wakeup");

    /**
     *  设置音量 0.0-1.0
     */
    void setVolume(float volume)
    {
        volume_config.volume = volume;
        // 如果你的 codec 本身声音偏大，可以在这里缩放一次，但值保持在 0~1.0 范围
#if defined(CODEC_TYPE_ES8311_NS4150) || defined(CODEC_TYPE_ES8311_ES7210)
        float adjusted = volume_config.volume * 0.1f; // 比如最多60%
        if(adjusted==0)
        {
            adjusted = 0.01f; // 最小音量
        }
        Serial.printf("setVolume: %f, adjusted: %f\n", volume_config.volume, adjusted);
        esp_ai_volume.setVolume(adjusted);
#else
        if(volume_config.volume == 0){ 
        esp_ai_volume.setVolume(0.01);
        }else{ 
        esp_ai_volume.setVolume(volume_config.volume);
        }
#endif 
        if (onVolumeCb != nullptr)
        {
            onVolumeCb(volume_config.volume);
        }
    };
    /**
     * 监听音量变化
     * 设置音量 0.0-1.0
     **/
    void onVolume(void (*func)(float volume)) { onVolumeCb = func; }

    /**
     * 手动设置 wifi账号/wifi密码/api_key/本地缓存数据，设置后会重新连接wifi
     * 设置成功会返回 true，失败返回 false
     *
     * 使用案例：
     *
     *  JSONVar data;
     *  data["wifi_name"] = "111";
     *  data["wifi_pwd"] = "111";
     *  data["api_key"] = "xxx";
     *  data["其他自定义数据"] = "xxx";
     *  esp_ai.setWifiConfig(data);
     */
    bool setWifiConfig(JSONVar data);

    /**
     * 接收到控制命令后的后调
     * @param {String} command_id 命令id
     * @param {String} data       其他数据
     **/
    void onEvent(void (*func)(const String &command_id, const String &data)) { onEventCb = func; }

    /**
     * 统一错误回调
     *  错误码  |  错误信息
     * ------------------
     *  002    |  服务端认证错误
     *  003    |  获取服务信息失败，说明 api_key 有问题
     */
    void onError(void (*func)(const String &code, const String &at_pos, const String &message)) { onErrorCb = func; };

    /**
     * 设备连接上wifi后的回调
     * device_ip 是局域网ip
     */
    void onConnectedWifi(void (*func)(const String &device_ip)) { onConnectedWifiCb = func; };

    /**
     * 板子连接不上时会启动热点并且调用本回调，收到这个回调说明该提示用户打开配网页面了
     * @url 配网地址, 有屏幕的情况下建议将 url 生成为二维码显示
     */
    void onAPInfo(void (*func)(const String &url, const String &ip, const String &ap_name)) { onAPInfoCb = func; };

    /**
     * 设备网络状态、与服务连接状态改变的回调
     * ----------------------------
     * 标志符    |  说明
     * ----------------------------
     * "0"      |  未连接wifi
     * "0_ing"  |  正在连接wifi(连接前)
     * "0_ing_after" |  正在连接wifi(连接后，会和连接前默认有 250ms 的延时)
     * "0_ap"   |  已经打开了配网热点，需要用户配网
     * "2"      |  未连接服务
     * "3"      |  已连接服务器
     */
    void onNetStatus(void (*func)(const String &status)) { onNetStatusCb = func; };

    /**
     * 设备会话状态回调
     * ----------------------------
     * 标志符        |  说明
     * ----------------------------
     * "xxx  "      |  休息状态
     * "iat_start"  |  开始进行语音识别
     * "iat_end"    |  语音识别结束
     * "tts_chunk_start"  |  tts 片段开始(单次对话会存在多次 tts_start)
     * "tts_chunk_end"    |  tts 片段结束 (单次对话会存在多次 tts_end)
     * "tts_real_end"     |  整个会话的 tts 真正结束
     * "llm_end"    |  llm 推理完毕 (推理完毕并不代表 tts 完毕)
     *
     */
    inline void onSessionStatus(void (*func)(const String &status)) { onSessionStatusCb = func; }

    /**
     * 设备准备就绪回调
     */
    inline void onReady(void (*func)()) { onReadyCb = func; }

    /**
     * 用户配网成功后会执行一次，开发者可以在本函数中发出设备绑定的请求
     * 比如用户填入 etx1(设定为你服务的 api_key) 后，服务端将 device_id 和 etx1 在业务服务仅绑定
     * data 参数是配网页面中给到 set_config 接口的数据，直接使用 data["xxx"] 来读取数据，如：data["wifi_name"]
     *
     *
     * 返回 true 则绑定设备成功，将会自动保存wifi等信息
     * 返回 false 则绑定设备失败，将提示用绑定设备失败的提示
     *
     * 返回 json 字符串， message 会直接在配网页面弹出
     * "{\"success\":false,\"message\":\"设备绑定失败，重启设备试试呢。\"}"
     * "{\"success\":true,\"message\":\"设备激活成功，即将重启设备。\"}"
     */
    void onBindDevice(String (*func)(JSONVar data)) { onBindDeviceCb = func; };

    /**
     * 获取存储在芯片中的数据
     * String ext1 = getLocalData("ext1");
     * 可读取的数据项 device_id |  wifi_name | wifi_pwd | api_key | 其他自定义参数
     */
    String getLocalData(const String &field_name) { return get_local_data(field_name); };

    /**
     * 设置存储在芯片中的数据
     * set_local_data("ext1", "自定义数据xxx");
     * 可设置的数据项 wifi_name | wifi_pwd | api_key | 其他自定义参数
     */
    void setLocalData(String field_name, String new_value) { set_local_data(field_name, new_value); };

    /**
     * 获取存储在芯片中的全部数据
     * JSONVar data = getLocalAllData();
     * Serial.println(data["wifi_name"]);
     */
    JSONVar getLocalAllData() { return get_local_all_data(); };

    /**
     * 手动控制设备输出语音
     */
    void tts(const String &text)
    {
        if (xSemaphoreTake(esp_ai_ws_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            JSONVar data;
            data["type"] = "tts";
            data["text"] = text;
            String send_data = JSON.stringify(data);
            esp_ai_webSocket.sendTXT(send_data);
            xSemaphoreGive(esp_ai_ws_mutex);
        }
    };

    /**
     * 停止会话, 表现为打断 TTS 等
     * 硬件端要停止会话时，必须用这个方法进行停止，而不是由服务进行终止
     */
    void stopSession()
    {
        DEBUG_PRINTLN(debug, F("[Info] -> 调用 SDK 停止会话"));
        esp_ai_start_ed = false;
        esp_ai_session_id = "";
        mp3_player_stop();
    };

    /**
     * 获取坐标定位
     * 利用设备连接的wifi或者ipv4地址，并且解析成为地址后返回
     * 硬件连接wifi后会执行一次
     *
     * @ip          wifi的公网临时ip
     * @nation      国家
     * @province    省份
     * @city        城市
     */
    void onPosition(void (*func)(const String &ip, const String &nation, const String &province, const String &city, const String &latitude, const String &longitude)) { onPositionCb = func; };

    /**
     * 按五次按钮回调，内部会自动执行清除配网信息的操作，
     * 如果开发者还需要执行其他操作，请在函数回调中执行
     */
    void onRepeatedlyClick(void (*func)()) { onRepeatedlyClickCb = func; };

    /**
     * 清除设备所有数据，包括开发者存储的数据和配网信息
     */
    void clearData() { clear_local_all_data(); };

    /**
     * 让设备重新进行一次音频缓存
     */
    void reCache();

    /**
     * 返回 false 则在 begin 时就停止程序执行，
     * 如检查到电池没电了，应该提示没电了，而不应该让程序继续执行下去
     */
    inline void onBegin(bool (*func)()) { onBeginCb = func; }

    /**
     * 情绪监听回调
     */
    inline void onEmotion(void (*func)(const String &emotion)) { onEmotionCb = func; }

    /**
     * 挂起所有 xTaskCreate 任务
     */
    void suspendAllTask();

    /**
     * 恢复所有 xTaskCreate 任务
     */
    void resumeAllTask();

    /**
     * 删除所有 xTaskCreate 任务
     */
    void delAllTask();

    /**
     * 获取设备是否正在播放音频
     */
    bool isSpeaking() { return mp3_player_is_playing(); };

    /**
     * 等待设备播放完毕
     */
    void awaitPlayerDone() { wait_mp3_player_done(); };

    /**
     * 播放 MP3 音频流
     * 16k采样率，16位深度，单声道音频
     */
    void playBuiltinAudio(const unsigned char *data, size_t len) { play_builtin_audio(data, len); };

private:
    ESP_AI_i2s_config_mic i2s_config_mic;
    ESP_AI_i2s_config_speaker i2s_config_speaker;
    ESP_AI_wifi_config wifi_config;
    ESP_AI_server_config server_config;
    ESP_AI_wake_up_config wake_up_config;
    ESP_AI_volume_config volume_config;
    ESP_AI_reset_btn_config reset_btn_config;
    ESP_AI_lights_config lights_config;
    bool debug;

    long send_start_time = 0;
    bool ready_ed = false;
    int asr_break_num = 0;

    void (*onReadyCb)() = nullptr;
    void (*onEventCb)(const String &command_id, const String &data) = nullptr;
    void (*onVolumeCb)(float volume) = nullptr;
    void (*onErrorCb)(const String &code, const String &at_pos, const String &message) = nullptr;
    void (*onAPInfoCb)(const String &url, const String &ip, const String &ap_name) = nullptr;
    void (*onNetStatusCb)(const String &status) = nullptr;
    void (*onConnectedWifiCb)(const String &device_ip) = nullptr;
    void (*onSessionStatusCb)(const String &status) = nullptr;
    void (*onPositionCb)(const String &ip, const String &nation, const String &province, const String &city, const String &latitude, const String &longitude) = nullptr;
    void (*onRepeatedlyClickCb)() = nullptr;
    bool (*onBeginCb)() = nullptr;
    void (*onEmotionCb)(const String &emotion) = nullptr;
    String (*onBindDeviceCb)(JSONVar data) = nullptr;

    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    int mic_i2s_init(uint32_t sampling_rate);
    void open_ap();

    void open_ble_server();
    void ble_connect_wifi();

    void connect_ws();

    void status_change(const String &status) { esp_ai_status = status; };

    void web_server_setCrossOrigin();
    void web_server_init();
    void web_server_page_index();
    void set_config();
    void get_config();

    void get_ssids();
    void re_scan_ssids();

    void clear_config();
    bool get_server_config();

    static void scan_wifi_wrapper(void *arg);
    void scan_wifi();

    static void reporting_sensor_data_wrapper(void *arg);
    void reporting_sensor_data();

    TaskHandle_t sensor_task_handle = NULL;
};
