
/**
* 本案例演示 使用按钮唤醒 ESP-AI 然后与之对话。也就是某个引脚高/低电平时唤醒或者打断会话
* 注意：配置值要用双引号！ 
*
* 三角按钮 | s3
*-----------
* OUT | 10
* GND | GND
* VCC | 5v（有些按钮可能是3.3v）
**/

#include <esp-ai.h>
ESP_AI esp_ai;

// 错误错误
void on_error(String code, String at_pos, String message) {
  Serial.printf("\n异常：%s -- %s -- %s\n", code, at_pos, message);
  // some code ...
}

// 统一状态捕捉
void on_net_status(String status) {
  Serial.printf("\n===设备网络状态===：%s", status);
  // some code ...
}

void setup() { 
  Serial.begin(115200);

  // [必  填] 是否调试模式， 会输出更多信息
  bool debug = true;
  // [必  填] wifi 配置： { wifi 账号， wifi 密码, "热点名字" } 可不设置，连不上wifi时会打开热点：ESP-AI，连接wifi后打开地址： 192.168.4.1 进行配网(控制台会输出地址，或者在ap回调中也能拿到信息) 
  ESP_AI_wifi_config wifi_config = { "", "", "ESP-AI" }; 
 
  // [必  填] 热点配置： 
  ESP_AI_server_config server_config = { };  

  // [必  填] 唤醒方案： { 方案, 语音唤醒用的阈值(本方案忽略即可), 引脚IO }
  ESP_AI_wake_up_config wake_up_config = { "pin_high", 1, 10 };  // 如果按钮按下是低电平，那使用 pin_low 即可 

  // 需要放到 begin 前面，否则可能监听不到一些数据
  esp_ai.onError(on_error);
  esp_ai.onNetStatus(on_net_status);

  esp_ai.begin({ debug, wifi_config, server_config, wake_up_config });

}

void loop() {
  esp_ai.loop();
}
