#include <esp-ai.h>

ESP_AI esp_ai;
// [必  填] 是否调试模式， 会输出更多信息
bool debug = true;
// [必  填] wifi 配置： { wifi 账号， wifi 密码 }  注意：要用双引号！
ESP_AI_wifi_config wifi_config = { "oldwang", "oldwang520" };
// [必  填] 服务配置： { 服务IP， 服务端口, "请求参数，用多个参数&号分割" }
ESP_AI_server_config server_config = { "192.168.1.5", 8080, "api-key=your_api_key&p2=test" };
// [必  填] 离线唤醒方案：{ 方案, 识别阈值 }, "edge_impulse" | "diy"，为 "diy" 时可调用 esp_ai.wakeUp() 方法进行唤醒
ESP_AI_wake_up_config wake_up_config = { "edge_impulse", 0.7 }; 

// [可留空] 麦克风引脚配置：{ bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic i2s_config_mic = { 4, 5, 6 };
// [可留空] 扬声器引脚配置：{ bck_io_num, ws_io_num, data_in_num, 采样率 }
ESP_AI_i2s_config_speaker i2s_config_speaker = { 16, 17, 15, 16000 };
// [可留空] 音量调节配置：{ 输入引脚，输入最大值(1024|4096)，默认音量(0-1) }
ESP_AI_volume_config volume_config = { 34, 4096, 0.5 };

// 收到指令后的回调，比如开灯、关灯，由服务端配置。 
void on_command(String command_id, String data) {
  Serial.printf("\n收到指令：%s -- %s\n", command_id, data);

  // 控制小灯演示
  // if (command_id == "device_open_001") {
  //   Serial.println("开灯");
  //   digitalWrite(led_pin, HIGH);
  // }
  // if (command_id == "device_close_001") {
  //   Serial.println("关灯");
  //   digitalWrite(led_pin, LOW);
  // }
}

void setup() {
  Serial.begin(115200);
  // 开始运行 ESP-AI
  esp_ai.begin({ i2s_config_mic, i2s_config_speaker, wifi_config, server_config, wake_up_config, volume_config, debug });
  // 用户指令监听
  esp_ai.onEvent(on_command);
}

void loop() {
  esp_ai.loop(); 
}