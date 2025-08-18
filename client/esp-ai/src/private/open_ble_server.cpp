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
const String EOT_MARKER = "--END--";
String ESP_AI_BLE_INCOMING_BUFFER = ""; // 初始化为空字符串
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
        Serial.println("wifi 连接失败");

        esp_ai_net_status = "0";
        ap_connect_err = "1";
        DEBUG_PRINTLN(debug, F("配网页面设置 WIFI 连接失败"));
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);
        vTaskDelay(pdMS_TO_TICKS(100));
        wait_mp3_player_done();
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 从新打开蓝牙，并且告知错误信息
        ESP_AI_BLE_ERR = "{\"success\":false,\"message\":\"wifi连接失败，请检查账号密码。\"}";
        clear_local_all_data(); // 清除临时数据
        // open_ble_server();
        ESP.restart();
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
        vTaskDelay(pdMS_TO_TICKS(100));
        // 重启板子
        wait_mp3_player_done();
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP.restart();
    }
    else
    {
        clear_local_all_data(); // 清除临时数据
        // open_ble_server();
        ESP.restart();
    }
}
 
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *esp_ai_ble_characteristic, NimBLEConnInfo &connInfo) override
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
            if (rxload.equals(EOT_MARKER))
            {
                if (ESP_AI_BLE_INCOMING_BUFFER.length() > 0)
                {
                    // 之前缓存的数据现在是完整的了
                    // 注意：您的 decodeURIComponent 函数需要能正确处理 ESP_AI_BLE_INCOMING_BUFFER
                    String decodedString = decodeURIComponent(ESP_AI_BLE_INCOMING_BUFFER);
                    ESP_AI_BLE_RD = decodedString; // 将解码后的完整数据存入 ESP_AI_BLE_RD
                    Serial.print(F("[BLE Recv] 完整数据接收并解码: "));
                    Serial.println(ESP_AI_BLE_RD);
                }
                else
                {
                    Serial.println(F("[BLE Recv] End marker received, but buffer was empty. No data to process."));
                }
                // 清空缓冲区，为下一条完整消息做准备
                ESP_AI_BLE_INCOMING_BUFFER = "";
            }
            else
            {
                // 这不是结束标记，是普通的数据块，追加到缓冲区
                ESP_AI_BLE_INCOMING_BUFFER += rxload;
                Serial.printf("[BLE Recv] Chunk appended. Total buffered length: %d\n", ESP_AI_BLE_INCOMING_BUFFER.length());
            }
        }
    }
} chrCallbacks;

void ESP_AI::open_ble_server()
{
#if !defined(DISABLE_BLE_NET)
    // 防止内存不足，先进行播报
    play_builtin_audio(wei_xin_pei_wang_mp3, wei_xin_pei_wang_mp3_len);
    awaitPlayerDone(); 

    String ap_name = get_ap_name(wifi_config.ap_name);
    NimBLEDevice::init(ap_name.c_str());
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    // NimBLEDevice::setMTU(128); // 更小的 MTU 减少内存占用
    NimBLEDevice::setSecurityAuth(false, false, false);
    esp_ai_ble_server = NimBLEDevice::createServer();
    esp_ai_ble_service = esp_ai_ble_server->createService(BLE_SERVICE_UUID);
    esp_ai_ble_characteristic = esp_ai_ble_service->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::NOTIFY);

    esp_ai_ble_characteristic->setCallbacks(&chrCallbacks);
    esp_ai_ble_service->start();
    esp_ai_ble_advertising = NimBLEDevice::getAdvertising(); // create advertising instance
    esp_ai_ble_advertising->setAppearance(0x00);             // 可选，设置设备外观

    NimBLEAdvertisementData advData;
    advData.setName(ap_name.c_str());
    esp_ai_ble_advertising->addServiceUUID(BLE_SERVICE_UUID);
    esp_ai_ble_advertising->setScanResponseData(advData);
    esp_ai_ble_advertising->setMinInterval(0x20); // 最小间隔 = 20 * 0.625ms = 12.5ms
    esp_ai_ble_advertising->setMaxInterval(0x40); // 最大间隔 = 40 * 0.625ms = 25ms
    esp_ai_ble_server->startAdvertising();
    esp_ai_ble_advertising->enableScanResponse(true);
    esp_ai_ble_advertising->start();
    if (ESP_AI_BLE_ERR != "")
    {
        delay(300);
        esp_ai_ble_characteristic->setValue(ESP_AI_BLE_ERR.c_str());
        esp_ai_ble_characteristic->notify();
    }
    else
    {
        esp_ai_ble_characteristic->setValue("HI ESP-AI");
        esp_ai_ble_characteristic->notify();
    }

    status_change("0_ap");
    if (onNetStatusCb != nullptr)
    {
        esp_ai_net_status = "0_ap";
        onNetStatusCb("0_ap");
    }
#else
    Serial.println(F("[Error] BLE 网络功能已禁用，请检查配置文件。"));
#endif
}