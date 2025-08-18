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

#include "main.h"

bool start_scan_ssids_ed = false;

void ESP_AI::set_config()
{
    String loc_wifi_name = get_local_data("wifi_name");
    String loc_wifi_pwd = get_local_data("wifi_pwd");
    String request_body = esp_ai_server.arg("plain"); // 读取原始请求体
    JSONVar data = JSON.parse(request_body);
    if (JSON.typeof(data) == "undefined")
    {
        DEBUG_PRINTLN(debug, ("传入数据解析失败或者传入了空数据。"));
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);
        web_server_setCrossOrigin();
        String json_response = "{\"success\":false,\"message\":\"传入数据解析失败或者传入了空数据。\"}";
        esp_ai_server.send(200, "application/json", json_response);
        return;
    }

    JSONVar keys = data.keys();
    String wifi_name = data["wifi_name"];
    String wifi_pwd = data["wifi_pwd"];

    for (int i = 0; i < keys.length(); i++)
    {
        String key = keys[i];
        JSONVar value = data[key];
        // Serial.print("设置 " + key + " : ");
        // Serial.println((const char *)value);
    }

    if (loc_wifi_name != wifi_name || loc_wifi_pwd != wifi_pwd)
    {
        // 为保证可以返回wifi连接状态。开启 Access Point 模式
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(wifi_name, wifi_pwd);
        DEBUG_PRINT(debug, F("connect wifi ing..."));
    }
    else
    {
        DEBUG_PRINT(debug, F("wifi信息并未发生变化，不重新连接wifi。仅进行重新绑定设备。"));
    }

    play_builtin_audio(lian_jie_zhong, lian_jie_zhong_len);
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
        DEBUG_PRINTLN(debug, ("配网页面设置 WIFI 连接失败"));
        play_builtin_audio(lian_jie_shi_bai, lian_jie_shi_bai_len);
        web_server_setCrossOrigin();
        String json_response = "{\"success\":false,\"message\":\"wifi连接失败，请检查账号密码，将会自动重启设备。\"}";
        esp_ai_server.send(200, "application/json", json_response);
        wait_mp3_player_done();
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
            Serial.println("\n[Error] onBindDeviceCb 回调函数返回数据格式有误，请检查 onBindDeviceCb 回调函数是否正常返回数据。");
        }
        is_bind_ok = (bool)parse_res["success"];

        // 配网和绑定设备回调都完毕了
        web_server_setCrossOrigin();
        esp_ai_server.send(200, "application/json", res_str);
    }
    else
    {
        // 到这里 wifi 连接已经成功了，如果是热点下，热点将会没了，这时候无法返回下面这个成功状态 ...
        web_server_setCrossOrigin();
        String json_response = "{\"success\":true,\"message\":\"wifi 连接成功，设备激活成功, 即将重启设备。\"}";
        esp_ai_server.send(200, "application/json", json_response);
        play_builtin_audio(pei_wang_cheng_gong, pei_wang_cheng_gong_len);
    }

    if (is_bind_ok)
    {
        JSONVar loc_data = get_local_all_data();
        String loc_wifi_name = loc_data["wifi_name"];
        String loc_wifi_name2 = loc_data["wifi_name2"];
        String loc_wifi_name3 = loc_data["wifi_name3"];
        String loc_wifi_name4 = loc_data["wifi_name4"];
        String loc_wifi_name5 = loc_data["wifi_name5"];
        String loc_wifi_pwd = loc_data["wifi_pwd"];
        String loc_wifi_pwd2 = loc_data["wifi_pwd2"];
        String loc_wifi_pwd3 = loc_data["wifi_pwd3"];
        String loc_wifi_pwd4 = loc_data["wifi_pwd4"];
        String loc_wifi_pwd5 = loc_data["wifi_pwd5"];
        if (wifi_name == loc_wifi_name2 && wifi_pwd == loc_wifi_pwd2)
        {
            data["wifi_name2"] = loc_wifi_name3;
            data["wifi_name3"] = loc_wifi_name4;
            data["wifi_name4"] = loc_wifi_name5;
            data["wifi_name5"] = "";
            data["wifi_pwd2"] = loc_wifi_pwd3;
            data["wifi_pwd3"] = loc_wifi_pwd4;
            data["wifi_pwd4"] = loc_wifi_pwd5;
            data["wifi_pwd5"] = "";
            keys = data.keys();
        }
        else if (wifi_name == loc_wifi_name3 && wifi_pwd == loc_wifi_pwd3)
        {
            data["wifi_name3"] = loc_wifi_name4;
            data["wifi_name4"] = loc_wifi_name5;
            data["wifi_name5"] = "";
            data["wifi_pwd3"] = loc_wifi_pwd4;
            data["wifi_pwd4"] = loc_wifi_pwd5;
            data["wifi_pwd5"] = "";
            keys = data.keys();
        }
        else if (wifi_name == loc_wifi_name4 && wifi_pwd == loc_wifi_pwd4)
        {
            data["wifi_name4"] = loc_wifi_name5;
            data["wifi_name5"] = "";
            data["wifi_pwd4"] = loc_wifi_pwd5;
            data["wifi_pwd5"] = "";
            keys = data.keys();
        }
        else if (wifi_name == loc_wifi_name5 && wifi_pwd == loc_wifi_pwd5)
        {
            data["wifi_name5"] = "";
            data["wifi_pwd5"] = "";
            keys = data.keys();
        }
        else if (loc_wifi_name != "")
        {
            // 去掉最旧的数据
            data["wifi_name2"] = loc_wifi_name;
            data["wifi_name3"] = loc_wifi_name2;
            data["wifi_name4"] = loc_wifi_name3;
            data["wifi_name5"] = loc_wifi_name4;
            data["wifi_pwd2"] = loc_wifi_pwd;
            data["wifi_pwd3"] = loc_wifi_pwd2;
            data["wifi_pwd4"] = loc_wifi_pwd3;
            data["wifi_pwd5"] = loc_wifi_pwd4;
            keys = data.keys();
        }

        for (int i = 0; i < keys.length(); i++)
        {
            String key = keys[i];
            JSONVar value = data[key];
            set_local_data(key, String((const char *)value));
        }

        // 重启板子
        wait_mp3_player_done();
        ESP.restart();
    }
}

void ESP_AI::get_config()
{
    String loc_device_id = get_device_id();
    JSONVar data = get_local_all_data();

    JSONVar keys = data.keys();
    for (int i = 0; i < keys.length(); i++)
    {
        String key = keys[i];
        Serial.print("获取 " + key + " : ");
        Serial.println((const char *)data[key]);
    }

    JSONVar json_response;
    json_response["success"] = true;
    data["device_id"] = loc_device_id;
    json_response["data"] = data;
    String send_data = JSON.stringify(json_response);

    web_server_setCrossOrigin();
    esp_ai_server.send(200, "application/json", send_data);
}

bool is_scan_ing = false;
bool is_scan_over = false;
JSONVar esp_ai_wifi_scan_json_response_data;

void ESP_AI::scan_wifi_wrapper(void *arg)
{
    ESP_AI *instance = static_cast<ESP_AI *>(arg);
    instance->scan_wifi();
}

void ESP_AI::scan_wifi()
{
    DEBUG_PRINTLN(debug, "[Info] 开始扫描wifi");
    is_scan_over = false;
    is_scan_ing = true;
    int n = WiFi.scanNetworks();
    DEBUG_PRINTLN(debug, "[Info] 扫描wifi完毕。");
    if (n == 0)
    {
        DEBUG_PRINTLN(debug, "[Info] 未搜索到任何 WIFI， 请重启开发板尝试。");
    }
    else
    {
        Serial.printf("[Info] 共找到 %d 个网络\n", n);
        for (int i = 0; i < n; ++i)
        {
            // 中国 2.4ghz 信道 1-14
            JSONVar json_item;
            json_item["ssid"] = WiFi.SSID(i).c_str();
            json_item["rssi"] = WiFi.RSSI(i);
            json_item["channel"] = WiFi.channel(i);
            esp_ai_wifi_scan_json_response_data[i] = json_item;
            // Serial.printf("网络 %d: %s, 信号强度: %d dBm\n", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
    }
    WiFi.scanDelete();
    is_scan_over = true;
    is_scan_ing = false;
    // 删除任务
    vTaskDelete(NULL);
}

void ESP_AI::re_scan_ssids()
{
    scan_wifi();
    JSONVar json_response;
    json_response["success"] = true;
    web_server_setCrossOrigin();
    String send_data = JSON.stringify(json_response);
    esp_ai_server.send(200, "application/json", send_data);
}

void ESP_AI::get_ssids()
{
    JSONVar json_response;
    json_response["success"] = true;
    if (is_scan_ing == false && is_scan_over == false)
    {
        xTaskCreate(ESP_AI::scan_wifi_wrapper, "scan_wifi", 1024 * 8, this, 1, NULL);
        json_response["status"] = "scaning";
        Serial.println("扫描为完毕，正在扫描中...");
    }
    else
    {
        Serial.println("扫描完毕");
        json_response["data"] = esp_ai_wifi_scan_json_response_data;
    }
    web_server_setCrossOrigin();
    String send_data = JSON.stringify(json_response);
    esp_ai_server.send(200, "application/json", send_data);
}

/***
 * 清除配网信息
 * 包括配网页面可配置的所有字段信息
 */
void ESP_AI::clear_config()
{
    clearData();
    web_server_setCrossOrigin();
    esp_ai_server.send(200, "application/json", "{\"success\":true,\"message\":\"清除配网信息成功, 即将重启设备。\"}");

    // 重启板子
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP.restart();
}

void ESP_AI::web_server_setCrossOrigin()
{
    esp_ai_server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    esp_ai_server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    esp_ai_server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    esp_ai_server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

void handleCaptivePortal()
{
    esp_ai_server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
    esp_ai_server.send(302, "text/plain", "");
}

void ESP_AI::web_server_init()
{
    esp_ai_server.on("/", [this]()
                     { this->web_server_page_index(); });

    esp_ai_server.on("/set_config", HTTP_POST, [this]()
                     { this->set_config(); });

    esp_ai_server.on("/get_config", [this]()
                     { this->get_config(); });

    esp_ai_server.on("/get_ssids", [this]()
                     { this->get_ssids(); });

    esp_ai_server.on("/re_scan_ssids", [this]()
                     { this->re_scan_ssids(); });

    esp_ai_server.on("/clear_config", [this]()
                     { this->clear_config(); });

    esp_ai_server.on("/generate_204", handleCaptivePortal);
    esp_ai_server.on("/fwlink", handleCaptivePortal);
    esp_ai_server.on("/hotspot-detect.html", handleCaptivePortal);
    esp_ai_server.onNotFound(handleCaptivePortal);

    esp_ai_server.begin();
}

const char esp_ai_html_str[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>

<head>
    <meta charset='UTF-8'>
    <meta name='viewport'
        content='viewport-fit=cover,width=device-width, initial-scale=1, maximum-scale=1, minimum-scale=1, user-scalable=no' />
    <title>ESP-AI 配网</title>
    <style>
        #ssid {
            border-radius: 3px;
            font-size: 14px;
            padding: 12px 12px;
            box-sizing: border-box;
            border: 1px solid #cfcfcf;
            outline: none;
            width: 100%;
        }

        #loading {
            position: fixed;
            z-index: 2;
            top: 0px;
            bottom: 0px;
            left: 0px;
            right: 0px;
            background-color: rgba(51, 51, 51, 0.8);
            text-align: center;
            color: #fff;
            font-size: 22px;
            transition: 0.3s;
        }

        #loading .icon {
            border: 16px solid #f3f3f3;
            border-radius: 50%;
            border-top: 16px solid #09aba2;
            border-bottom: 16px solid #40cea5;
            width: 120px;
            height: 120px;
            margin: auto;
            position: absolute;
            bottom: 0px;
            top: 0px;
            left: 0px;
            right: 0px;
            -webkit-animation: spin 2s linear infinite;
            animation: spin 2s linear infinite;
        }

        .body {
            height: 100vh;
            margin: 0px;
            font-size: 16px;
            color: #333;
            position: relative;
            font-family: 苹方, 黑体;
            overflow: hidden;
        }

        .logo {
            text-align: left;
            width: 100%;
            padding: 24px 24px 50px 24px;
            box-sizing: border-box;
        }

        .desc {
            width: 100%;
            font-size: 12px;
            padding: 6px 24px;
            padding-bottom: 32px;
            box-sizing: border-box;
            color: #929292;
            text-align: left; 
        }

        .code {
            width: 100%;
            font-size: 13px;
            color: #929292;
            text-align: center;
            padding: 24px;
            box-sizing: border-box;
        }

        @-webkit-keyframes spin {
            0% {
                -webkit-transform: rotate(0deg);
            }

            100% {
                -webkit-transform: rotate(360deg);
            }
        }

        @keyframes spin {
            0% {
                transform: rotate(0deg);
            }

            100% {
                transform: rotate(360deg);
            }
        }
    </style>
</head>

<body class='body'>
    <div style='height: 100%;display: flex;flex-direction: column;' id='wifi_setting_panel'>
        <div class='logo'>
            <a href='https://espai.fun'
                style='display: flex;align-items: center;font-size: 12px; color: #09aba2;text-decoration: none;'>
                <img style='width:24px; margin-right: 8px;'
                    src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABABAMAAABYR2ztAAAABGdBTUEAALGPC/xhBQAAAAFzUkdCAK7OHOkAAAAwUExURT/UoUK8tf7//0DIqkG/sD/Jp+vu7kWxlFOojtXd27HGwT7HmGaml3WqnZC0rIKunvd5/rkAAAOcSURBVEjHhZVdaBRXFMcPC7XatQ8bhErwITmbnZ3dJBvYm7FUVEgYEvuiMtNpjKUQxrj5AEEn1G0CgowfRMiDLhgYC7KsOoX6siy02O6LbF9aCH1IoaBPxT6IiE9CoQXBc++d2ezsJtn/w7135vzmnHPPnJkL2EWww/3MrsBviEq8XmoD+labKtLlU4BfokAt6qUP4MMIMIjFUD/z6xwARICbw82HbRkcwG4B0qW2EFgGcFuAoe/at1IFcEKAXNViHZWobgGkWBR4Wd9EpfqiBWj18Jq80yZLO1WyDFJxOwRsd5MU2nMQKvSQ3Uf6eB9mfyAhzkjr3vLe4QDoC3gx11Ak8OQV/h0bDICn+zdIsJbxvAzE8SrZn70jix2UWimLDcBzdXX11m3ADM8PlpU4BkC2HHtcr7vwPwG+Cz+JNw1Q/UiFmADUqzVedvhWKRQW529DJSBifRRPhJiJCcCnHLK+B/GbiCMS4G8LcLAqgTrhT9R5lTJYoeW8S8AesYsZQ3pIF68MNnyXF/Krf3kHAI9BHk4bC55ng4YFu/FaHbkoqvLB1DPeMgb3cM7g22F+yjoznGu4D5imaYyxM8iY9ikBmbuWRUvrFd7zjK8fKz9aJmOWZZbO08Bq5CElAcTkZ3kz6zySgJHhFDvOq2UYRt7II1rThnksTQHI8qViWXxVEgCZDQPTbNr5lVV4fMacczRo7DC2ACZrONfYZWFnKMY8BgDZjSXGVlioo2orUOFaLZpsS/lkE6BXpAjJhvtHApVrYhrlAGOpJJepTa7wNl3aSoGd5SHeakklY192bvBiWKO/I/7JRlVejC/+szmQ1JRx9/7i3BD13eY3VGIbl46kRLWmZEdp07Nq5o2/7HleiQCLNdA5L8sp+gGXTGe8kJujbAsFW9RvAk3TohWTXZ3SpmaVhRfi30GVMOnhTUvjHs5KYEAfc5aVNO/9jUVdaHxATsGnp+uT6XWbRyj8tbExS5YTEpgIgIe6/rknUij4xeJDfVLXUQAnA4Dj3/six3tiDIETAcCd9opCKy7t1VtfH3BljK0QgU5Rqm/W1v7o74kA15uAzn86+gHKSRWXdriLDo2hHCUwluiUzW8e3AY4dEHO4/2JxET4j4o86uAlPvX0J3pxWyDhojJ2wI+cOG3hKx1HUhvwSeeZNRQlejoPtZFuAGJ/NwCHuwGId7oBTSc9O5+8d7oB0sluAOa6AYhzCUd0X5fTv6n31Vew2Q+oLzsAAAAASUVORK5CYII=' />
                ESP_AI - 为万物赋予灵魂
            </a>
        </div>
        <div style='flex: 1;'></div>
        <div style='padding-bottom: 18px;text-align: center;'>
            <div style='font-size: 22px;color: #757575;'>
                ESP-AI 设备配网
            </div>
        </div>

        <div style='width: 100%;padding: 8px 24px;box-sizing: border-box;'>
            <select id='ssid' style='background: #fff;padding-right: 20px;' placeholder='请选择 wifi'>
                <option value='' id='scan-ing'>网络扫描中...</option>
            </select>
        </div>
        <div style='width: 100%;padding: 8px 24px;box-sizing: border-box;'>
            <input id='password' name='password' type='text' placeholder='请输入WIFI密码'
                style='border-radius: 3px;font-size:14px;padding: 12px 12px;box-sizing: border-box;border: 1px solid #cfcfcf;outline: none;width:100%;'>
        </div>
        <div style='width: 100%;padding: 8px 24px;box-sizing: border-box;'>
            <input id='api_key' name='api_key' type='text' placeholder='[可选] 开放平台秘钥'
                style='border-radius: 3px;font-size:14px;padding: 12px 12px;box-sizing: border-box;border: 1px solid #cfcfcf;outline: none;width:100%;'>
            </div>
            <div class='desc'>输入开放平台秘钥后将直接使用开放平台接口，如需使用本地服务，请在硬件代码中写入您的服务器信息。</div>
        <div style='width: 100%;text-align: right; padding: 6px 24px; padding-bottom: 32px; box-sizing: border-box;'>
            <button id='submit-btn'
                style='width:100% ; box-sizing: border-box; border-radius: 3px; padding: 16px 0px;border: none;color: #fff;background-color: transparent; color:#fff; background: linear-gradient(45deg, #40cea5, #09aba2); box-shadow: 0px 0px 3px #ccc;letter-spacing: 2px;'>连接WIFI</button>
        </div>

        <div style='flex: 1;'></div>
        <div class='desc' style='line-height: 1.5;'> 
            <b>秘钥获取方式：</b>
            <ul style='padding-left: 0px; list-style: none;'>
                <li>1. 打开网址 http://dev.espai.fun</li>
                <li>2. 创建超体</li>
                <li>3. 复制左下角 api_key 黏贴到本页的秘钥输入框</li>
            </ul>

        </div>
        <div class='code'>
            设备编码：<span id='device_id'> - </span>
        </div>
    </div>
    <div id='loading'>
        <div class='icon'></div>
    </div>
</body>

</html>

<script>
    var domain = '';
    var scan_time = null;
    var loading = false;
    var scan = false;
    function myFetch(apiName, params, cb, method = 'GET') { 
        const url = domain + apiName;  
        const options = {
            method: method, 
            mode: 'cors',
            headers: {
                'Content-Type': 'application/json'  
            }
        }; 
        if (method.toUpperCase() === 'POST' || method.toUpperCase() === 'PUT') {
            options.body = JSON.stringify(params);  
        } 
        fetch(url, options)
            .then(function (res) { return res.json() })
            .then(function (data) {
                cb && cb(data);
            });
    };

    function get_config() {
        myFetch('/get_config', {}, function (res) {
            document.querySelector('#loading').style.display = 'none';
            if (res.success) {
                var data = res.data;
                if (data.wifi_name) {
                    document.querySelector('#ssid').value = data.wifi_name;
                }
                if (data.wifi_pwd) {
                    document.querySelector('#password').value = data.wifi_pwd;
                }
                if (data.api_key) {
                    document.querySelector('#api_key').value = data.api_key;
                }
                if (data.device_id) {
                    document.querySelector('#device_id').innerHTML = data.device_id;
                }
            } else {
                alert('获取配置失败, 请刷新页面重试');
            }
        });
    }

    function scan_ssids() {
        console.log('获取 wifi');
        myFetch('/get_ssids', {}, function (res) {
            if (res.success) {
                if (res.status === 'scaning') {
                    setTimeout(scan_ssids, 1000);
                    return;
                }
                var data = res.data || [];
                if (data.length > 0) {
                    document.querySelector('#scan-ing').innerHTML = '请选择wifi...';
                };
                var selectDom = document.getElementById('ssid');
                var options = selectDom.getElementsByTagName('option') || [];
                var optionsName = [];
                for (var i = 0; i < options.length; i++) {
                    optionsName.push(options[i].getAttribute('value'));
                };
                data.forEach(function (item) {
                    if (item.ssid && !optionsName.includes(item.ssid)) {
                        var option = document.createElement('option');
                        option.innerText = item.ssid + '     (' + (item.channel <= 14 ? '2.4GHz' : '5GHz') + ' 信道：' + item.channel + ')';
                        option.setAttribute('value', item.ssid);
                        selectDom.appendChild(option);
                    }
                });
                if (data.length) {
                    get_config();
                    clearInterval(scan_time);
                };
            } else {
                alert('启动网络扫描失败，请刷新页面重试');
            }
        });
    }


    function setWifiInfo() {
        if (loading) {
            return;
        };
        var wifi_name = document.querySelector('#ssid').value;
        var wifi_pwd = document.querySelector('#password').value;
        var api_key = document.querySelector('#api_key').value;
        if (!wifi_name) {
            alert('请输入 WIFI 账号哦~');
            return;
        }
        if (!wifi_pwd) {
            alert('请输入 WIFI 密码哦~');
            return;
        }
        loading = true;
        document.querySelector('#submit-btn').innerHTML = '配网中...';
        clearTimeout(window.reloadTimer);
        window.reloadTimer = setTimeout(function () {
            alert('未知配网状态，即将重启设备');
            setTimeout(function () {
                window.location.reload();
            }, 2000);
        }, 20000);
        
        
        myFetch('/set_config', {
            wifi_name: wifi_name,
            wifi_pwd: wifi_pwd,
            api_key: api_key
            }, function(res) {
                clearTimeout(window.reloadTimer);
                loading = false;
                document.querySelector('#submit-btn').innerHTML = '连接WIFI';

                if (res.success) {
                    alert(res.message);
                    window.close();
                } else {
                    alert(res.message);
                }
        }, 'POST'); 
    };


    window.onload = function () {
        scan_ssids();
        document.querySelector('#submit-btn').addEventListener('click', setWifiInfo);
    }
</script>
    )rawliteral";
void ESP_AI::web_server_page_index()
{
    web_server_setCrossOrigin(); 
    if (wifi_config.html_str[0] != '\0')
    {
        esp_ai_server.send_P(200, "text/html", wifi_config.html_str);
    }
    else
    {
        esp_ai_server.send_P(200, "text/html", esp_ai_html_str);
    }
}