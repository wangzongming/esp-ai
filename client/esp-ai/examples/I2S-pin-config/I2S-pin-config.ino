
/**
 * 本案例演示 自定义I2S引脚
 * 注意：配置值要用双引号！ 
 **/

#include <esp-ai.h>

ESP_AI esp_ai;

void setup()
{
  Serial.begin(115200);

  // [必  填] 是否调试模式， 会输出更多信息
  bool debug = true;
  // [必  填] wifi 配置： { wifi 账号， wifi 密码, "热点名字" } 可不设置，连不上wifi时会打开热点：ESP-AI，连接wifi后打开地址： 192.168.4.1 进行配网(控制台会输出地址，或者在ap回调中也能拿到信息)
  ESP_AI_wifi_config wifi_config = { "", "", "ESP-AI"};
  // [可 填] 服务配置： { 服务协议, 服务IP， 服务端口, "[可选] 请求参数 " }
  ESP_AI_server_config server_config = {};
  ESP_AI_wake_up_config wake_up_config = {"edge_impulse", 0.95};

  // [可留空] 麦克风引脚配置：{ bck_io_num, ws_io_num, data_in_num }
  ESP_AI_i2s_config_mic i2s_config_mic = {4, 5, 6};
  // [可留空] 扬声器引脚配置：{ bck_io_num, ws_io_num, data_in_num, 采样率 }
  ESP_AI_i2s_config_speaker i2s_config_speaker = {16, 17, 15, 16000};
  // [可留空] 音量调节配置：{ 输入引脚，输入最大值(1024|4096)，默认音量(0-1) }
  ESP_AI_volume_config volume_config = {34, 4096, 0.8};

  esp_ai.begin({debug, wifi_config, server_config, wake_up_config, volume_config, i2s_config_mic, i2s_config_speaker});
}

void loop()
{
  esp_ai.loop();
}
