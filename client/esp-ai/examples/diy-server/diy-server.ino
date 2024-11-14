
/**
 * 本案例演示: 连接自定义服务和固定 wifi
 * 注意：配置值要用双引号！
 * 服务端使用：https://github.com/wangzongming/esp-ai/tree/master/example
 **/

#include <esp-ai.h>

ESP_AI esp_ai;

void setup()
{
  Serial.begin(115200);
  // [必  填] 是否调试模式， 会输出更多信息
  bool debug = true;
  // [必  填] wifi 配置： { wifi 账号， wifi 密码, "热点名字" } 可不设置，连不上wifi时会打开热点：ESP-AI，连接wifi后打开地址： 192.168.4.1 进行配网(控制台会输出地址，或者在ap回调中也能拿到信息)
  // ESP_AI_wifi_config wifi_config = {"test", "12345678", "ESP-AI"};
  ESP_AI_wifi_config wifi_config = {"oldwang", "oldwang520", "ESP-AI"};
  // [可 填] 服务配置： { 服务协议, 服务IP， 服务端口, "[可选] 请求参数" }
  ESP_AI_server_config server_config = {"http", "192.168.3.23", 8088};
  // [必  填] 唤醒方案： { 方案, 语音唤醒用的阈值(本方案忽略即可), 引脚唤醒方案(本方案忽略), 发送的字符串 }
  ESP_AI_wake_up_config wake_up_config = { "asrpro", 1, 10, "start" };   
  // 启动
  esp_ai.begin({debug, wifi_config, server_config, wake_up_config });
}

void loop()
{
  esp_ai.loop();
}
