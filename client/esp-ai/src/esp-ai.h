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

/***
 * RAM
 * |   task                  |     occupy     |
 * |-------------------------|----------------|
 * |   long press            |     -kb        |
 * |   capture samples       |     -kb       |
 * |   wakeup inference      |     -kb       |
 * |   i2s_listener          |     -kb       |
 * |   volume                |     -kb        |
 * |   ws2812                |     -kb        |
 * |   reporting sensor data |     -kb        |
 *
 *
 */

#pragma once
#include "globals.h"

class ESP_AI
{
public:
    ESP_AI();
    void begin(ESP_AI_CONFIG config);
    void loop();
    bool wifiIsConnected();
    std::string localIP();
    void wakeUp(String scene = "wakeup");
    // 设置音量 0-1
    void setVolume(float volume);

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
    void onEvent(void (*func)(String command_id, String data));

    /**
     * 统一错误回调
     *  错误码  |  错误信息
     * ------------------
     *  002    |  服务端认证错误
     *  003    |  获取服务信息失败，说明 api_key 有问题
     */
    void onError(void (*func)(String code, String at_pos, String message));

    /**
     * 设备连接上wifi后的回调
     * device_ip 是局域网ip
     */
    void onConnectedWifi(void (*func)(String device_ip));

    /**
     * 板子连接不上时会启动热点并且调用本回调，收到这个回调说明该提示用户打开配网页面了
     * @url 配网地址, 有屏幕的情况下建议将 url 生成为二维码显示
     */
    void onAPInfo(void (*func)(String url, String ip, String ap_name));

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
    void onNetStatus(void (*func)(String status)); 

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
    void onSessionStatus(void (*func)(String status));

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
    void onBindDevice(String (*func)(JSONVar data));

    /**
     * 获取存储在芯片中的数据
     * String ext1 = getLocalData("ext1");
     * 可读取的数据项 device_id |  wifi_name | wifi_pwd | api_key | 其他自定义参数
     */
    String getLocalData(String field_name);

    /**
     * 设置存储在芯片中的数据
     * set_local_data("ext1", "自定义数据xxx");
     * 可设置的数据项 wifi_name | wifi_pwd | api_key | 其他自定义参数
     */
    void setLocalData(String field_name, String new_value);

    /**
     * 获取存储在芯片中的全部数据
     * JSONVar data = getLocalAllData(); 
     * Serial.println(data["wifi_name"]);
     */
    JSONVar getLocalAllData();

    /**
     * 手动控制设备输出语音
     */
    void tts(String text);

    /**
     * 停止会话, 表现为打断 TTS 等
     * 硬件端要停止会话时，必须用这个方法进行停止，而不是由服务进行终止
     */
    void stopSession();

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
    void onPosition(void (*func)(String ip, String nation, String province, String city));

    /**
     * 按五次按钮回调，内部会自动执行清除配网信息的操作，
     * 如果开发者还需要执行其他操作，请在函数回调中执行
     */
    void onRepeatedlyClick(void (*func)());

    /**
     * 清除设备所有数据，包括开发者存储的数据和配网信息
     */
    void clearData();

private:
    ESP_AI_i2s_config_mic i2s_config_mic;
    ESP_AI_i2s_config_speaker i2s_config_speaker;
    ESP_AI_wifi_config wifi_config;
    ESP_AI_server_config server_config;
    ESP_AI_wake_up_config wake_up_config;
    ESP_AI_volume_config volume_config;
    ESP_AI_reset_btn_config reset_btn_config;
    bool debug;

    void (*onEventCb)(String command_id, String data) = nullptr;
    void (*onErrorCb)(String code, String at_pos, String message) = nullptr;
    void (*onAPInfoCb)(String url, String ip, String ap_name) = nullptr;
    void (*onNetStatusCb)(String status) = nullptr;
    void (*onConnectedWifiCb)(String device_ip) = nullptr;
    void (*onSessionStatusCb)(String status) = nullptr;
    void (*onPositionCb)(String ip, String nation, String province, String city) = nullptr;
   
    String (*onBindDeviceCb)(JSONVar data) = nullptr;
    void (*onRepeatedlyClickCb)() = nullptr;

    void speaker_i2s_setup();
    void adjustVolume(int16_t *buffer, size_t length, float volume);
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    int mic_i2s_init(uint32_t sampling_rate);
    void open_ap();
    void connect_ws();

    static void volume_listener_wrapper(void *arg);
    void volume_listener();

    static void lights_wrapper(void *arg);
    void lights();

    bool microphone_inference_start(uint32_t n_samples);
    void microphone_inference_end(void);

    static void capture_samples_wrapper(void *arg);
    void capture_samples();

    void wakeup_init();
    static void wakeup_inference_wrapper(void *arg);
    void wakeup_inference();

    static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr);
    void status_change(String status);

    void web_server_setCrossOrigin();
    void web_server_init();
    void web_server_page_index();
    void set_config();
    void get_config();

    void get_ssids();
    void re_scan_ssids();

    void clear_config();
    bool get_server_config();

    static void get_position_wrapper(void *arg);
    void get_position();

    static void scan_wifi_wrapper(void *arg);
    void scan_wifi();

    static void on_repeatedly_click_wrapper(void *arg);
    void on_repeatedly_click();

    static void send_audio_wrapper(void *arg);
    void send_audio();

    static void play_audio_wrapper(void *arg);
    void play_audio();

    void audio_inference_callback(uint32_t n_bytes);
    int i2s_deinit(void);

    static void reporting_sensor_data_wrapper(void *arg);
    void reporting_sensor_data();
};
