#include <esp-ai.h>

ESP_AI esp_ai;
// [必  填] 是否调试模式， 会输出更多信息
bool debug = true;
// [必  填] 服务配置： { 服务IP， 服务端口 }
ESP_AI_server_config server_config = { "192.168.1.5", 8080 };
// [必  填] 离线唤醒方案：{ 方案, 识别阈值 }, "edge_impulse" | "diy"，为 "diy" 时可调用 esp_ai.wakeUp() 方法进行唤醒
ESP_AI_wake_up_config wake_up_config = { "edge_impulse", 0.7 };

// [可留空] 麦克风引脚配置：{ bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic i2s_config_mic = {};
// [可留空] 扬声器引脚配置：{ bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_speaker i2s_config_speaker = {};
// [可留空] 音量调节配置：{ 输入引脚，输入最大值(1024|4096)，默认音量(0-1) }
ESP_AI_volume_config volume_config = { 34, 4096, 0.5 };

// 收到指令后的回调，比如开灯、关灯，由服务端配置。
void on_command(char command_id, char data) {
  Serial.print("收到指令");
  Serial.println(command_id);
  Serial.println(data);
}

void setup() {
  Serial.begin(115200);
  // 开始运行 ESP-AI
  esp_ai.begin({ i2s_config_mic, i2s_config_speaker, server_config, wake_up_config, volume_config, debug });
  // wifi 测试代码
  // Serial.println(esp_ai.wifiIsConnected() ? "已连接" : "未连接");
  // Serial.println(esp_ai.localIP().c_str());
  // 用户指令监听
  esp_ai.onEvent(on_command);
}

void loop() {
  // 连接wifi后会返回 True
  if (!esp_ai.wifiIsConnected()) {
    return;
  } 
  esp_ai.loop();
}