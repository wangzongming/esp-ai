
/**
 * 本案例演示 自定义配网页面
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

void setup()
{
  Serial.begin(115200);

  // [必  填] 是否调试模式， 会输出更多信息
  bool debug = true;
  // [必  填] wifi 配置： { wifi 账号， wifi 密码, "热点名字" } 可不设置，连不上wifi时会打开热点：ESP-AI，连接wifi后打开地址： 192.168.4.1 进行配网(控制台会输出地址，或者在ap回调中也能拿到信息)
  ESP_AI_wifi_config wifi_config = {"", "", "ESP-AI"};
  // 自定义配网页面
  char html_str[] = "HTML content";
  wifi_config.html_str = (char *)malloc(strlen(html_str) + 1);
  strcpy(wifi_config.html_str, html_str);

  ESP_AI_server_config server_config = {};
  ESP_AI_volume_config volume_config = {34, 4096, 0.7};

  // [必  填] 唤醒方案： { 方案, 语音唤醒用的阈值(本方案忽略即可), 引脚IO }
  ESP_AI_wake_up_config wake_up_config = {};
  strcpy(wake_up_config.wake_up_scheme, "asrpro"); // 唤醒方案
  strcpy(wake_up_config.str, "start");             // 串口和天问asrpro 唤醒时需要配置的字符串，也就是从另一个开发版发送来的字符串

  esp_ai.begin({debug, wifi_config, server_config, wake_up_config, volume_config});
}

void loop()
{
  esp_ai.loop();
}
