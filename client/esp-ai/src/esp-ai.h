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
    void wakeUp();
    // 设置音量 0-100
    void setVolume(int volume);
    
    /**
     * 手动设置 wifi账号/wifi密码/api_key/ext1/ext2 配置，设置后会重新连接wifi
     * 除了 wifi 账号和和密码外其他都是可选的，不改是传入空字符串即可
     * 设置成功会返回 true，失败返回 false
    */
    bool setWifiConfig(char wifi_name[60], char wifi_pwd[60], char api_key[60], char ext1[60], char ext2[60]);

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
     * "0_ing"  |  正在连接wifi
     * "0_ap"   |  已经打开了配网热点，需要用户配网 
     * "2"      |  未连接服务
     * "3"      |  已连接服务器 
    */
    void onNetStatus(void (*func)(String status));
    
private:
    ESP_AI_i2s_config_mic i2s_config_mic;
    ESP_AI_i2s_config_speaker i2s_config_speaker;
    ESP_AI_wifi_config wifi_config;
    ESP_AI_server_config server_config;
    ESP_AI_wake_up_config wake_up_config;
    ESP_AI_volume_config volume_config;
    bool debug; 
    void (*onEventCb)(String command_id, String data) = nullptr;  
    void (*onErrorCb)(String code, String at_pos, String message) = nullptr;  
    void (*onAPInfoCb)(String url, String ip, String ap_name) = nullptr;  
    void (*onNetStatusCb)(String status) = nullptr;  
    void (*onConnectedWifiCb)(String device_ip) = nullptr;   

    void speaker_i2s_setup();
    void adjustVolume(int16_t *buffer, size_t length, float volume);
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    int mic_i2s_init(uint32_t sampling_rate);
    bool microphone_inference_start(uint32_t n_samples);
    void microphone_inference_end(void);
    void capture_samples(void *arg);

    static void capture_samples_wrapper(void *arg); 

    void wakeup_init();
    void wakeup_inference();
    static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr);

    void web_server_setCrossOrigin();
    void web_server_init();
    void web_server_page_index();
    void set_config();
    void get_config();
 
    bool get_server_config(); 

    void audio_inference_callback(uint32_t n_bytes);
    int i2s_deinit(void);
};
