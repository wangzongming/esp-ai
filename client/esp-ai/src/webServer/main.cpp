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

#include "main.h"

void ESP_AI::set_config()
{
    String wifi_name = server.arg("wifi_name");
    String wifi_pwd = server.arg("wifi_pwd");
    String api_key = server.arg("api_key");
    String ext1 = server.arg("ext1");
    String ext2 = server.arg("ext2");

    DEBUG_PRINTLN(debug, "================== Set Config ====================");
    DEBUG_PRINTLN(debug, "设置 wifi_name：" + wifi_name);
    DEBUG_PRINTLN(debug, "设置 wifi_pwd：" + wifi_pwd);
    DEBUG_PRINTLN(debug, "设置 api_key：" + api_key);
    DEBUG_PRINTLN(debug, "设置 ext1：" + ext1);
    DEBUG_PRINTLN(debug, "设置 ext2：" + ext2);
    DEBUG_PRINTLN(debug, "===================================================");

    WiFi.begin(wifi_name, wifi_pwd);
    DEBUG_PRINT(debug, "connect wifi ing..");
    int connect_count = 0;
    // 10s 连不上Wifi的话就判定失败
    int try_count = 20;
    while (WiFi.status() != WL_CONNECTED && connect_count <= try_count)
    {
        connect_count++;
        delay(500);
        DEBUG_PRINT(debug, ".");
    }
    DEBUG_PRINTLN(debug, "");
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN(debug, "设置 WIFI 连接失败");
        ESP.restart();
        web_server_setCrossOrigin();
        String json_response = "{\"success\":false,\"message\":\"wifi连接失败，请检查账号密码\"}";
        server.send(200, "application/json", json_response);
        return;
    }

    set_local_data("wifi_name", wifi_name);
    set_local_data("wifi_pwd", wifi_pwd);
    set_local_data("api_key", api_key);
    set_local_data("ext1", ext1);
    set_local_data("ext2", ext2);

    web_server_setCrossOrigin();
    String json_response = "{\"success\":true,\"message\":\"设置成功, 即将重启设备。\"}";
    server.send(200, "application/json", json_response);

    // 重启板子
    delay(100);
    ESP.restart();
}

void ESP_AI::get_config()
{
    String loc_device_id = get_local_data("device_id");
    String loc_wifi_name = get_local_data("wifi_name");
    String loc_wifi_pwd = get_local_data("wifi_pwd");
    String loc_api_key = get_local_data("api_key");
    String loc_ext1 = get_local_data("ext1");
    String loc_ext2 = get_local_data("ext2");

    JSONVar json_response;
    JSONVar json_response_data;
    json_response["success"] = true;
    json_response_data["device_id"] = loc_device_id;
    json_response_data["wifi_name"] = loc_wifi_name;
    json_response_data["wifi_pwd"] = loc_wifi_pwd;
    json_response_data["api_key"] = loc_api_key;
    json_response_data["ext1"] = loc_ext1;
    json_response_data["ext2"] = loc_ext2;
    json_response["data"] = json_response_data;
    String send_data = JSON.stringify(json_response);

    web_server_setCrossOrigin();
    server.send(200, "application/json", send_data);
}

void ESP_AI::web_server_setCrossOrigin()
{
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

void ESP_AI::web_server_init()
{ 
    server.on("/", [this]()
              { this->web_server_page_index(); });
    server.on("/set_config", [this]()
              { this->set_config(); });
    server.on("/get_config", [this]()
              { this->get_config(); });
    server.begin();
}

void ESP_AI::web_server_page_index()
{
    String content = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport'content='viewport-fit=cover,width=device-width, initial-scale=1, maximum-scale=1, minimum-scale=1, user-scalable=no'/><title>ESP-AI配网</title></head><body style='margin: 0;font-size: 16px; color: #333;height: 80vh;position: relative;font-family: 苹方,黑体;overflow: hidden;'><div style='height: 100%;display: flex;flex-direction: column; justify-content: center;align-items: center;'id='wifi_setting_panel'><div style='padding-bottom: 18px;text-align: center;'><img style='width:60px;'src='data:image/png;base64,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'/><div style='font-size: 22px;color: #757575;'>为ESP-AI设备配网</div></div><div style='width: 100%;padding: 8px 24px;box-sizing: border-box;'><input id='ssid'name='ssid'type='text'placeholder='请输入WIFI账号，仅支持2.4GHZ频段WIFI'style='border-radius: 3px;font-size:14px;padding: 12px 12px;box-sizing: border-box;border: 1px solid #cfcfcf;outline: none;width:100%;'></div><div style='width: 100%;padding: 8px 24px;box-sizing: border-box;'><input id='password'name='password'type='text'placeholder='请输入WIFI密码'style='border-radius: 3px;font-size:14px;padding: 12px 12px;box-sizing: border-box;border: 1px solid #cfcfcf;outline: none;width:100%;'></div><div style='width: 100%;padding: 8px 24px;box-sizing: border-box;'><input id='api_key'name='api_key'type='text'placeholder='请输入产品密钥, 请到开发者平台获取'style='border-radius: 3px;font-size:14px;padding: 12px 12px;box-sizing: border-box;border: 1px solid #cfcfcf;outline: none;width:100%;'></div><div style='width: 100%;text-align: right; padding: 6px 24px; padding-bottom: 32px; box-sizing: border-box;'><button id='submit-btn'style='width:100% ; box-sizing: border-box; border-radius: 3px; padding: 16px 0px;border: none;color: #fff;background-color: transparent; color:#fff; background: linear-gradient(45deg, #40cea5, #09aba2); box-shadow: 0px 0px 3px #ccc;letter-spacing: 2px;'>连接WIFI</button></div><div style='font-size: 13px;position: fixed;bottom: 12px;color:#929292'>设备编码：<span id='device_id'>-</span></div></div></body></html><script>var domain='';var loading=false;function myFetch(apiName,params,cb){fetch(domain+apiName,{mode:'cors'}).then(function(res){return res.json()}).then(function(data){cb&&cb(data)})};function refreshInfo(){myFetch('/get_config',{},function(res){console.log('wifi信息：',res);if(res.success){var data=res.data;document.querySelector('#ssid').value=data.wifi_name;document.querySelector('#password').value=data.wifi_pwd;document.querySelector('#api_key').value=data.api_key;document.querySelector('#device_id').innerHTML=data.device_id}else{alert('获取配置失败, 请刷新页面重试')}})};function setWifiInfo(){if(loading){return};var wifi_name=document.querySelector('#ssid').value;var wifi_pwd=document.querySelector('#password').value;var api_key=document.querySelector('#api_key').value;if(!wifi_name){alert('请输入 WIFI 账号哦~');return}if(!wifi_pwd){alert('请输入 WIFI 密码哦~');return}loading=true;document.querySelector('#submit-btn').innerHTML='配网中...';clearTimeout(window.reloadTimer);window.reloadTimer=setTimeout(function(){window.location.reload()},10000);myFetch('/set_config?wifi_name='+wifi_name+'&wifi_pwd='+wifi_pwd+'&api_key='+api_key,{},function(res){clearTimeout(window.reloadTimer);loading=false;document.querySelector('#submit-btn').innerHTML='连接WIFI';if(res.success){alert('配网成功，即将重启设备，关闭本页面即可');}else{alert('配网失败，请检查账号密码是否正确')}})};window.onload=function(){refreshInfo();document.querySelector('#submit-btn').addEventListener('click',setWifiInfo)}</script>";

    if(String(wifi_config.html_str) != ""){
        content = String(wifi_config.html_str);
    }
 
    web_server_setCrossOrigin(); // 避免跨域
    server.send(200, "text/html", content);
}