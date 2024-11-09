#include <esp-ai.h>

/**
 * 本案例演示 控制小灯（用户指令解析）
 *
 **/

ESP_AI esp_ai;

// 控制小灯演示
int led_pin = 18;

// 收到指令后的回调，比如开灯、关灯，由服务端配置。
void on_command(String command_id, String data)
{
  Serial.printf("\n收到指令：%s -- %s\n", command_id, data);

  // 控制小灯演示
  if (command_id == "device_open_001")
  {
    Serial.println("开灯");
    digitalWrite(led_pin, HIGH);
  }
  if (command_id == "device_close_001")
  {
    Serial.println("关灯");
    digitalWrite(led_pin, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);

  // [必  填] 是否调试模式， 会输出更多信息
  bool debug = true;
  // [必  填] wifi 配置： { wifi 账号， wifi 密码, "热点名字" } 可不设置，连不上wifi时会打开热点：ESP-AI，连接wifi后打开地址： 192.168.4.1 进行配网(控制台会输出地址，或者在ap回调中也能拿到信息)
  ESP_AI_wifi_config wifi_config = {"", "", "ESP-AI"};
  // [可 填] 服务配置： { 服务协议, 服务IP， 服务端口, "[可选] 请求参数，用免费服务时，在 dev.espai.fun 中复制 key 即可" }
  ESP_AI_server_config server_config = {};
  // [必  填] 离线唤醒方案：{ 方案, 识别阈值 }, "edge_impulse" | "diy"，为 "diy" 时可调用 esp_ai.wakeUp() 方法进行唤醒
  ESP_AI_wake_up_config wake_up_config = {"edge_impulse", 0.95};

  // 开始运行 ESP-AI
  esp_ai.begin({i2s_config_mic, i2s_config_speaker, wifi_config, server_config, wake_up_config, volume_config, debug});
  // 用户指令监听
  esp_ai.onEvent(on_command);
}

void loop()
{
  esp_ai.loop();
}