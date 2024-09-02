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
    if (config.i2s_config_mic.bck_io_num)
    {
        i2s_config_mic = config.i2s_config_mic;
    }
    if (config.i2s_config_speaker.bck_io_num)
    {
        i2s_config_speaker = config.i2s_config_speaker;
    }
    if (strcmp(config.wifi_config.wifi_name, "") != 0 || strcmp(config.wifi_config.ap_name, "") != 0 || strcmp(config.wifi_config.html_str, "") != 0)
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

    if (strcmp(config.wake_up_config.wake_up_scheme, "") != 0)
    {
        wake_up_config = config.wake_up_config;
        wake_up_scheme = String(wake_up_config.wake_up_scheme);
        if (wake_up_scheme == "pin_high" || wake_up_scheme == "pin_low")
        {
            pinMode(wake_up_config.pin, INPUT);
        }
    }
    if(wake_up_scheme == "edge_impulse"){
        Serial.println("[Error] edge_impulse 唤醒方案在 ESP-AI v2.0.0 中暂未发布，预计2.2.x中进行发布，请先使用其它唤醒方案！");
        return;
    }

    // led 指示灯
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // 初始化EEPROM，总容量为512字节
    EEPROM.begin(512);
    DEBUG_PRINTLN(debug, "");

    delay(1000);

    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        net_status = "0";
        onNetStatusCb("0");
    }

    String loc_is_ready = get_local_data("is_ready");
    String loc_device_id = get_local_data("device_id");
    String loc_wifi_name = get_local_data("wifi_name");
    String loc_wifi_pwd = get_local_data("wifi_pwd");
    String loc_api_key = get_local_data("api_key");
    String loc_ext1 = get_local_data("ext1");
    String loc_ext2 = get_local_data("ext2");
    DEBUG_PRINTLN(debug, ("==================== Local Data ===================="));
    DEBUG_PRINTLN(debug, "loc_is_ready: " + loc_is_ready);
    if (loc_is_ready != "ok")
    {
        // 始化 rom 中存在乱码
        EEPROM.put(0, "");
        EEPROM.commit();
        String uuid = generateUUID();
        String init_ok = "ok";
        set_local_data("is_ready", init_ok);
        set_local_data("device_id", uuid);
        DEBUG_PRINTLN(debug, "初始化 device_id：" + uuid);
        loc_is_ready = init_ok;
        loc_device_id = uuid;
    }
    DEBUG_PRINTLN(debug, "loc_device_id: " + loc_device_id);
    DEBUG_PRINTLN(debug, "loc_wifi_name: " + loc_wifi_name);
    DEBUG_PRINTLN(debug, "loc_wifi_pwd: " + loc_wifi_pwd);
    DEBUG_PRINTLN(debug, "loc_api_key: " + loc_api_key);
    DEBUG_PRINTLN(debug, "loc_ext1: " + loc_ext1);
    DEBUG_PRINTLN(debug, "loc_ext2: " + loc_ext2);
    DEBUG_PRINTLN(debug, ("====================================================="));

    // 启动热点，用于配网
    if (loc_wifi_name == "" && wifi_config.wifi_name == "")
    { 
        WiFi.mode(WIFI_AP);
        String ap_name = strlen(wifi_config.ap_name) > 0 ? wifi_config.ap_name : "ESP-AI";
        WiFi.softAP(ap_name);
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        String httpUrl = "http://" + ipStr;
        DEBUG_PRINTLN(debug, "\nWIFI名称：" + ap_name);
        DEBUG_PRINTLN(debug, "配网地址：" + httpUrl);
        // 启动配网服务
        web_server_init();
        if (onAPInfoCb != nullptr)
        {
            onAPInfoCb(httpUrl, ipStr, ap_name);
        }
        return;
    }

    DEBUG_PRINTLN(debug, ("==================== Connect WIFI ===================="));
    String _wifi_name = loc_wifi_name;
    String _wifi_pwd = loc_wifi_pwd;
    if (_wifi_name == "")
    {
        _wifi_name = wifi_config.wifi_name;
    }
    if (_wifi_pwd == "")
    {
        _wifi_name = wifi_config.wifi_pwd;
    }

    DEBUG_PRINTLN(debug, "wifi name: " + _wifi_name);
    DEBUG_PRINTLN(debug, "wifi pwd: " + _wifi_pwd);
    WiFi.begin(_wifi_name, _wifi_pwd);
    DEBUG_PRINT(debug, F("connect wifi ing.."));

    int connect_count = 0;
    // 10s 连不上Wifi的话
    int try_count = 20;
    while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
    {
        connect_count++;
        delay(500);
        DEBUG_PRINT(debug, ".");
        // 设备状态回调
        if (onNetStatusCb != nullptr)
        {
            net_status = "0_ing";
            onNetStatusCb("0_ing");
        }
        if (connect_count > try_count)
        {
            DEBUG_PRINTLN(debug, ("\n连接WIFI失败，请重新配网"));
            // 重新配网 
            WiFi.mode(WIFI_AP);
            String ap_name = strlen(wifi_config.ap_name) > 0 ? wifi_config.ap_name : "ESP-AI";
            WiFi.softAP(ap_name);
            IPAddress ip = WiFi.softAPIP();
            String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
            String httpUrl = "http://" + ipStr;
            DEBUG_PRINTLN(debug, "WIFI名称：" + ap_name);
            DEBUG_PRINTLN(debug, "配网地址：" + httpUrl);
            // 启动配网服务
            web_server_init();
            if (onAPInfoCb != nullptr)
            {
                onAPInfoCb(httpUrl, ipStr, ap_name);
            }
            // 设备状态回调
            if (onNetStatusCb != nullptr)
            {
                net_status = "0_ap";
                onNetStatusCb("0_ap");
            }
        }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        net_status = "2";
        onNetStatusCb("2");
    }

    // 不休眠，不然可能播放不了
    WiFi.setSleep(false);

    DEBUG_PRINTLN(debug, "");
    DEBUG_PRINT(debug, F("wifi 连接成功，设备 IP 地址: "));
    DEBUG_PRINTLN(debug, WiFi.localIP());
    String ip_str = WiFi.localIP().toString(); 
    if (onConnectedWifiCb != nullptr)
    { 
        onConnectedWifiCb(ip_str);
    }

    DEBUG_PRINTLN(debug, ("==============================================="));

    if (wake_up_scheme == "edge_impulse")
    {
        DEBUG_PRINTLN(debug, ("=================== Edge Impulse ================"));
        wakeup_init();
        DEBUG_PRINTLN(debug, ("==============================================="));
    }
    else
    {
        // 其他情况也正常初始化麦克风，需要上传音频给 IAT
        if (mic_i2s_init(16000))
        {
            DEBUG_PRINTLN(debug, ("[Error] Failed to start I2S!"));
        }
    }
 
    speaker_i2s_setup(); 
    
    bool get_server_success = get_server_config(); 
    if(get_server_success == false){
        DEBUG_PRINTLN(debug, ("[Error] 服务配置获取失败!"));
        return;
    } 

    // 为方便更改，在局域网也建立服务
    web_server_init();  
    webSocket.begin(
        server_config.ip,
        server_config.port,
        "/?v=" + ESP_AI_VERSION +
            "&device_id=" + loc_device_id +
            "&api_key=" + loc_api_key +
            "&ext1=" + loc_ext1 +
            "&ext2=" + loc_ext2 +
            "&" + server_config.params);

    webSocket.onEvent(std::bind(&ESP_AI::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
