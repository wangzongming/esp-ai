#include <esp-ai.h>

/**
 * 本案例演示 自定义方式唤醒：例如用 按钮唤醒小明同学 
 *
 * 按钮接线
 * Btn  s3
 * OUT => 8
 * GND => GND
 * VCC => 5v（有些按钮可能不是5v哦！）
 **/

ESP_AI esp_ai;

int btnPin = 10;  // 按钮引脚
int btnClick = 0; // 按钮是否按下：1按下 0否

long lastDebounceTime = 0; // 最后一次按下按钮的时间
long debounceDelay = 300;  // 节流

void setup()
{
  Serial.begin(115200);
  pinMode(btnPin, INPUT);

  // [必  填] 是否调试模式， 会输出更多信息
  bool debug = true;
  // [必  填] wifi 配置： { wifi 账号， wifi 密码, "热点名字" } 可不设置，连不上wifi时会打开热点：ESP-AI，连接wifi后打开地址： 192.168.4.1 进行配网(控制台会输出地址，或者在ap回调中也能拿到信息)
  ESP_AI_wifi_config wifi_config = {"", "", "ESP-AI"};
  // [可 填] 服务配置： { 服务协议, 服务IP， 服务端口, "[可选] 请求参数 }
  ESP_AI_server_config server_config = {};
  // [必  填] 离线唤醒方案：{ 方案, 识别阈值 }, "edge_impulse" | "diy"，为 "diy" 时可调用 esp_ai.wakeUp() 方法进行唤醒
  ESP_AI_wake_up_config wake_up_config = {"diy"};

  // 开始运行 ESP-AI
  esp_ai.begin({debug, wifi_config, server_config, wake_up_config});
}

void loop()
{
  esp_ai.loop();

  int reading = digitalRead(btnPin);
  // Serial.println(reading);
  long curTime = millis();
  // 不同的三角按钮原理图这里可能不一样，案例代码是按下为高电平
  if (reading == 1)
  {
    if ((curTime - lastDebounceTime) > debounceDelay)
    {
      Serial.println("按下了按钮");
      // 按下按钮后唤醒 小明同学
      esp_ai.wakeUp();
      lastDebounceTime = curTime;
    }
  }
}