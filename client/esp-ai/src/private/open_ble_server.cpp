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
#include "open_ble_server.h"

BLEServer *esp_ai_ble_server;
BLECharacteristic *esp_ai_ble_characteristic;
BLEService *esp_ai_ble_service;
BLEAdvertising *esp_ai_ble_advertising;

 
/**
 * 蓝牙连接回调
 */

typedef void (*ConnectCallback)(void *arg);
typedef void (*DisconnectCallback)(void *arg);
class MyServerCallbacks : public BLEServerCallbacks
{
private:
    ConnectCallback connectCallback;
    DisconnectCallback disconnectCallback;

public:
    // 构造函数，用于传入连接和断开时要调用的函数
    MyServerCallbacks(ConnectCallback connect, DisconnectCallback disconnect)
        : connectCallback(connect), disconnectCallback(disconnect) {}

    void onConnect(BLEServer *pServer)
    {
        // 调用连接时的回调函数
        if (connectCallback)
        {
            connectCallback(&pServer);
        }
    };

    void onDisconnect(BLEServer *pServer)
    {
        // 断开后需要重新开始广播，不然就再次连接就会提示 超时
        // https://github.com/espressif/arduino-esp32/issues/6016
        pServer->startAdvertising();
        // 调用断开时的回调函数
        if (disconnectCallback)
        {
            disconnectCallback(&pServer);
        }
    }
};

/**
 * 接收到特征值的回调
 */
typedef void (*OnWriteCb)(void *arg);
class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
private:
    OnWriteCb onWriteCb;

public:
    // 构造函数，用于传入连接和断开时要调用的函数
    CharacteristicCallbacks(OnWriteCb writeCb) : onWriteCb(writeCb) {}
    void onWrite(BLECharacteristic *esp_ai_ble_characteristic)
    {
        std::string rxValue = esp_ai_ble_characteristic->getValue();
        if (rxValue.length() > 0)
        {
            String rxload = "";
            for (int i = 0; i < rxValue.length(); i++)
            {
                if (rxValue[i])
                {
                    rxload += (char)rxValue[i];
                }
            }
            ESP_AI_BLE_RD = rxload;
            // 调用连接时的回调函数
            if (onWriteCb)
            {
                onWriteCb(&esp_ai_ble_characteristic);
            }
        }
    }
};

void ESP_AI::on_ble_device_connected_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->on_ble_device_connected();
}
void ESP_AI::on_ble_device_connected()
{
    DEBUG_PRINTLN(debug, "[Info] 设备已连接蓝牙服务");
}

void ESP_AI::on_ble_device_disconnected_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->on_ble_device_disconnected();
}
void ESP_AI::on_ble_device_disconnected()
{
    DEBUG_PRINTLN(debug, "[Info] 设备已断开蓝牙服务");
}

void ESP_AI::characteristic_callbacks_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->characteristic_callbacks();
}
void ESP_AI::characteristic_callbacks()
{
    // DEBUG_PRINTLN(debug, "[Info] 接收到特征值");
    // DEBUG_PRINTLN(debug, ESP_AI_BLE_RD);
    String decodedString = decodeURIComponent(ESP_AI_BLE_RD);
    DEBUG_PRINTLN(debug, "[Info] 收到蓝牙数据");
    DEBUG_PRINTLN(debug, decodedString);
    JSONVar data = JSON.parse(decodedString);

    String loc_wifi_name = get_local_data("wifi_name");
    String loc_wifi_pwd = get_local_data("wifi_pwd");
    if (JSON.typeof(data) == "undefined")
    {
        DEBUG_PRINTLN(debug, ("传入数据解析失败或者传入了空数据。"));
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);

        String json_response = "{\"success\":false,\"message\":\"传入数据解析失败或者传入了空数据。\"}";
        esp_ai_ble_characteristic->setValue(json_response.c_str());
        esp_ai_ble_characteristic->notify();
        return;
    }

    // 将数据都全部存入本地
    play_builtin_audio(lian_jie_zhong, lian_jie_zhong_len);
    JSONVar keys = data.keys();
    for (int i = 0; i < keys.length(); i++)
    {
        String key = keys[i];
        JSONVar value = data[key];
        set_local_data(key, String((const char *)value));
    }
    // 记录这是蓝牙临时数据
    set_local_data("_ble_temp_", "1");
    // 重启板子
    wait_mp3_player_done();
    ESP.restart(); 
}

void ESP_AI::ble_connect_wifi()
{
    JSONVar data = get_local_all_data();
    String wifi_name = (const char *)data["wifi_name"];
    String wifi_pwd = (const char *)data["wifi_pwd"]; 
 
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_name, wifi_pwd);
    DEBUG_PRINT(debug, F("connect wifi ing..."));
 

    ap_connect_err = "0";
    int connect_count = 0;
    // 10s 连不上Wifi的话就判定失败
    int try_count = 15;
    while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
    {
        connect_count++;
        // 内置状态处理
        status_change("0_ing");
        // 设备状态回调
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing");
        }
        delay(250);
        DEBUG_PRINT(debug, ".");
        // 内置状态处理
        status_change("0_ing_after");
        if (onNetStatusCb != nullptr)
        {
            esp_ai_net_status = "0_ing";
            onNetStatusCb("0_ing_after");
        }
        delay(250);
    }

    // 这里连接失败后是无法给用户提示，因为wifi断开了，ip地址也没了，所以只能重启板子
    // 如果失败，连接时间将会 >= 7500ms, 其他情况都只能说明 wifi 连接成功了
    if (WiFi.status() != WL_CONNECTED)
    {
        esp_ai_net_status = "0";
        ap_connect_err = "1";
        DEBUG_PRINTLN(debug, F("配网页面设置 WIFI 连接失败"));
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);

        // 从新打开蓝牙，并且告知错误信息
        ESP_AI_BLE_ERR = "{\"success\":false,\"message\":\"wifi连接失败，请检查账号密码。\"}";
        clear_local_all_data(); // 清除临时数据
        open_ble_server();
        return;
    }

    // 执行绑定设备的方法
    bool is_bind_ok = true;
    if (onBindDeviceCb != nullptr)
    {
        String loc_device_id = get_device_id();
        data["device_id"] = loc_device_id;
        String res_str = onBindDeviceCb(data);

        JSONVar parse_res = JSON.parse(res_str);
        if (JSON.typeof(parse_res) == "undefined")
        {
            is_bind_ok = false;
            Serial.println(F("\n[Error] onBindDeviceCb 回调函数返回数据格式有误，请检查 onBindDeviceCb 回调函数是否正常返回数据。"));
        }
        is_bind_ok = (bool)parse_res["success"];
        ESP_AI_BLE_ERR = res_str;
    }
    if (is_bind_ok)
    {
        set_local_data("_ble_temp_", "0"); // 清除临时标识
        play_builtin_audio(pei_wang_cheng_gong, pei_wang_cheng_gong_len);
        // 重启板子
        wait_mp3_player_done();
        ESP.restart();
    }
    else
    {
        clear_local_all_data(); // 清除临时数据
        open_ble_server();
    }
}

void ESP_AI::open_ble_server()
{
    String ap_name = get_ap_name(wifi_config.ap_name);
    BLEDevice::init(ap_name.c_str());
    esp_ai_ble_server = BLEDevice::createServer();
    // 服务连接回调
    esp_ai_ble_server->setCallbacks(new MyServerCallbacks(ESP_AI::on_ble_device_connected_wrapper, ESP_AI::on_ble_device_disconnected_wrapper));
    // 创建一个服务
    esp_ai_ble_service = esp_ai_ble_server->createService(BLE_SERVICE_UUID);
    // 创建特征，注意，PROPERTY_READ PROPERTY_WRITE PROPERTY_NOTIFY 这三个都得启用
    esp_ai_ble_characteristic = esp_ai_ble_service->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

    // 监听客户端改变特征值的回调
    esp_ai_ble_characteristic->setCallbacks(new CharacteristicCallbacks(ESP_AI::characteristic_callbacks_wrapper));
    // 改变特征值，客户端可以监听到特征值的改变
    esp_ai_ble_characteristic->setValue("HI ESP-AI");
    // 启动服务
    esp_ai_ble_service->start();

    // 固定写法
    esp_ai_ble_advertising = BLEDevice::getAdvertising();
    esp_ai_ble_advertising->addServiceUUID(BLE_SERVICE_UUID);
    esp_ai_ble_advertising->setScanResponse(true);
    esp_ai_ble_advertising->setMinPreferred(0x06); // 帮助解决iPhone连接问题的功能
    esp_ai_ble_advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    DEBUG_PRINTLN(debug, "[Info] 蓝牙服务器启动成功");
    play_builtin_audio(qing_pei_wang, qing_pei_wang_len);

    // 发送特征值
    // esp_ai_ble_characteristic->setValue("hello");
    // esp_ai_ble_characteristic->notify();
    if (ESP_AI_BLE_ERR != "")
    {
        delay(300);
        esp_ai_ble_characteristic->setValue(ESP_AI_BLE_ERR.c_str());
        esp_ai_ble_characteristic->notify();
    }

    // 内置状态处理
    status_change("0_ap");
    // 设备状态回调
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "0_ap";
        onNetStatusCb("0_ap");
    }
}