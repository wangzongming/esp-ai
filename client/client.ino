
/**
* 作者： 小明IO
* 开源地址： https://github.com/wangzongming/esp-ai
* 如果不需要语音唤醒功能，不需要 esp32s3 也能运行起来本代码（两个 I2S 接口必须拥有即可）
* 开发板子使用 esp32s3
* 
* INMP441	ESP32s3	  备注
* VDD	    3v3	    
* GND	    GND	    GND
* L/R	    GND	    左声道或右声道(低电平时左声道输出，反之右声道输出)
* WS	    5      左时钟 右时钟
* SCK	    4	    串行时钟
* SD	    6	    串行数据
*
*
* Max98357A	ESP32s3	  备注
* VDD	    3v3	     
* GND	    GND	    GND 
* LRC	    17      
* BCLK	  16	    
* DIN	    15	    
*
* LED	ESP32s3	  备注
* vcc    18     家电控制演示
*
*
* 电位器	ESP32s3	  备注
* OUT    34        音量调节
*
* 模型使用  Edge Impulse 制作
* 开源仓库： https://studio.edgeimpulse.com/studio/422422
**/

#include <driver/i2s.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
// 音频流播放插件
#include "AudioTools.h"

// 模型插件
#include <xiao_ming_tong_xue_inferencing.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// 固定设置
#define SAMPLE_BUFFER_SIZE 512

// ============================ 可改配置 ======================
// 大多数麦克风可能默认为左通道，但可能需要将L/R引脚绑低
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
// 麦克风引脚
#define I2S_MIC_BIN 4
#define I2S_MIC_WIN 5
#define I2S_MIC_DIN 6
#define MIC_i2s_num I2S_NUM_1

// 扬声器引脚
#define I2S_DOUT 15
#define I2S_BCLK 16
#define I2S_LRC 17
#define YSQ_i2s_num I2S_NUM_0
// wifi信息
const char *ssid = "oldwang";
const char *password = "oldwang520";           // 欢迎来蹭我网~~~
const char *websocket_server = "192.168.1.5";  // 替换为WebSocket服务器地址
const uint16_t websocket_port = 8080;          // 替换为WebSocket服务器端口
const char *websocket_path = "/";              // 替换为WebSocket服务器路径

// led引脚
int led_pin = 18;
// 电位器引脚
int ctrl_btn = 34;
// 电位器最大值
int ctrl_max_val = 4095;
// ==================================================


// 当前电位器值
int cur_ctrl_val = 0;
// 默认音量
float volume = 0.5;
// 离线语音 “小明同学” 识别阈值  0-1
float voice_key_threshold = 0.7;

WebSocketsClient webSocket;

String start_ed = "0";

// 是否可以采集音频，由客户端控制的
String can_voice = "1";

// 最后一次接收到音频流的时间
long last_get_audio_time = 0;
// 是否已经通知服务端播放完毕
String is_send_server_audio_over = "1";
// 超过多少时间没有接收到音频后就算结束播放 ms
long audio_delay = 600;

/** 音频缓冲区，指针和选择器 */
typedef struct {
  int16_t *buffer;
  uint8_t buf_ready;
  uint32_t buf_count;
  uint32_t n_samples;
} inference_t;
static inference_t inference;
static const uint32_t sample_buffer_size = 1024;
// static const uint32_t sample_buffer_size = 2048;
// static const uint32_t sample_buffer_size = 4096;
static signed short sampleBuffer[sample_buffer_size];
// 设置为true以查看从原始信号生成的特征
static bool debug_nn = false;
static bool record_status = true;


// 初始化麦克风 IsS
static int i2s_init(uint32_t sampling_rate) {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = sampling_rate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_MIC_CHANNEL,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_BIN,
    .ws_io_num = I2S_MIC_WIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_DIN,
  };


  esp_err_t ret = 0;
  ret = i2s_driver_install(MIC_i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin(MIC_i2s_num, &i2s_mic_pins);

  if (ret != ESP_OK) {
    ei_printf("Error in i2s_driver_install");
  }

  if (ret != ESP_OK) {
    ei_printf("Error in i2s_set_pin");
  }

  ret = i2s_zero_dma_buffer(MIC_i2s_num);
  if (ret != ESP_OK) {
    ei_printf("Error in initializing dma buffer with 0");
  }

  return int(ret);
}


// 扬声器 I2S 配置
I2SStream i2s;
void speaker_i2s_setup() {
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // 配置项文档
  // https://pschatzmann.github.io/arduino-audio-tools/classaudio__tools_1_1_i2_s_config_e_s_p32.html
  // esp32 代码配置处，其他的板子自行查询
  // https://github.com/pschatzmann/arduino-audio-tools/blob/9045503daae3b21300ee7bb76c4ad95efe9e1e6c/src/AudioI2S/I2SESP32.h#L186
  auto config = i2s.defaultConfig(TX_MODE);
  config.sample_rate = 16000;
  config.bits_per_sample = 16;
  config.port_no = YSQ_i2s_num;  // 这里别和麦克风冲突了，esp32 有两个可用通道
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.i2s_format = I2S_MSB_FORMAT;  // 这里最好看看源码中的类型定义
  config.buffer_count = 8;
  config.buffer_size = 1024;
  config.auto_clear = true;
  config.channels = 1;
  config.pin_ws = I2S_LRC;     //LCK
  config.pin_bck = I2S_BCLK;   //BCK
  config.pin_data = I2S_DOUT;  //DIN
  i2s.begin(config);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  // 不休眠，不然可能播放不了
  WiFi.setSleep(false);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // 关键词模型相关信息和设置
  ei_printf("Inferencing settings:\n");
  ei_printf("\tInterval: ");
  ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
  ei_printf(" ms.\n");
  ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
  ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));
  ei_printf("2秒后开始连续推理…");
  ei_sleep(2000);

  if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
    ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
    return;
  }


  // led 指示灯
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // 演示 led 灯
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // 启动扬声器
  speaker_i2s_setup();

  // ws 服务启动
  webSocket.begin(websocket_server, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
}


int32_t raw_samples[SAMPLE_BUFFER_SIZE];
void loop() {
  webSocket.loop();

  // ing...
  // size_t bytes_read;
  // uint16_t buffer[1024] = {0};
  // i2s_read(MIC_i2s_num, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
  // signal_t signal;
  // signal.total_length = bytes_read;
  // signal.get_data = (uint8_t *)buffer;
  // ei_impulse_result_t result = { 0 };
  // EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);



  // buf 准备好后就进行推理
  if (inference.buf_ready != 0 && can_voice == "1") {
    // 重置 buf 状态
    inference.buf_ready = 0;
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", r);
      return;
    }
    float xmtx_val = result.classification[2].value;
    // Serial.print(xmtx_val);
    if (xmtx_val >= voice_key_threshold) {
      Serial.println("");
      Serial.print("小明同学：");
      Serial.print(xmtx_val);
      Serial.println("");

      // led 打开
      digitalWrite(LED_BUILTIN, HIGH);

      // 开始录音
      start_ed = "1";
      webSocket.sendTXT("start");
      Serial.println("开始录音");
    }
  }

  // 扬声器音量调节逻辑
  int _cur_ctrl_val = analogRead(ctrl_btn);
  if (_cur_ctrl_val != cur_ctrl_val) {
    cur_ctrl_val = _cur_ctrl_val;
    // 计算最新音量
    volume = static_cast<float>(cur_ctrl_val) / ctrl_max_val;
    Serial.println(volume);
  }

  // 是否播放完毕
  long cur_time = millis();
  if (is_send_server_audio_over == "0" && (cur_time - last_get_audio_time > audio_delay)) {
    is_send_server_audio_over = "1";
    digitalWrite(LED_BUILTIN, LOW);
    webSocket.sendTXT("client_out_audio_over");
  }
}

// 调整音量
void adjustVolume(int16_t *buffer, size_t length, float volume) {
  for (size_t i = 0; i < length / 2; i++) {  // 每次处理两个字节
    buffer[i] = (int16_t)(buffer[i] * volume);
  }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      can_voice = "1";
      start_ed = "0";
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("WebSocket Disconnected");
      break;
    case WStype_CONNECTED:
      can_voice = "1";
      start_ed = "0";
      Serial.println("WebSocket Connected");
      break;
    case WStype_TEXT:
      // 一边用喇叭，一边采集音频会很卡
      if (strcmp((char *)payload, "start_voice") == 0) {
        can_voice = "1";
        Serial.print("继续采集音频");
        // webSocket.sendTXT("start");
      }
      if (strcmp((char *)payload, "pause_voice") == 0) {
        can_voice = "0";
        Serial.print("暂停采集音频");
      }
      if (strcmp((char *)payload, "iat_end") == 0) {
        Serial.println("当前语音识别完毕啦！");
      }
      if (strcmp((char *)payload, "session_end") == 0) {
        start_ed = "0";
        can_voice = "1";
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("会话结束");
      }

      /** 可修改的业务代码 **/
      // 控制演示
      if (strcmp((char *)payload, "device_open_001") == 0) {
        Serial.println("开灯");
        digitalWrite(led_pin, HIGH);
        can_voice = "1";
      }
      if (strcmp((char *)payload, "device_close_001") == 0) {
        Serial.println("关灯");
        digitalWrite(led_pin, LOW);
        can_voice = "1";
      }


      Serial.printf("Received Text: %s\n", payload);
      break;
    case WStype_BIN:
      {
        last_get_audio_time = millis();
        if (is_send_server_audio_over != "0") {
          is_send_server_audio_over = "0";
        }
        // 调整音量
        adjustVolume((int16_t *)payload, length, volume);
        // 输出
        i2s.write(payload, length);

        break;
      }
    case WStype_PING:
      Serial.println("Ping");
      break;
    case WStype_PONG:
      Serial.println("Pong");
      break;
  }
}



static bool microphone_inference_start(uint32_t n_samples) {
  inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

  if (inference.buffer == NULL) {
    return false;
  }

  inference.buf_count = 0;
  inference.n_samples = n_samples;
  inference.buf_ready = 0;

  if (i2s_init(EI_CLASSIFIER_FREQUENCY)) {
    ei_printf("Failed to start I2S!");
  }

  ei_sleep(100);

  record_status = true;

  xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void *)sample_buffer_size, 10, NULL);

  return true;
}


static bool microphone_inference_record(void) {
  bool ret = true;

  while (inference.buf_ready == 0) {
    delay(10);
  }

  inference.buf_ready = 0;
  return ret;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
  numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

  return 0;
}

static void microphone_inference_end(void) {
  i2s_deinit();
  ei_free(inference.buffer);
}


static void capture_samples(void *arg) {

  const int32_t i2s_bytes_to_read = (uint32_t)arg;
  size_t bytes_read = i2s_bytes_to_read;

  while (record_status) {
    i2s_read(MIC_i2s_num, (void *)sampleBuffer, i2s_bytes_to_read, &bytes_read, portMAX_DELAY);
    // 发送给服务端
    if (start_ed == "1" && can_voice == "1") {
      if (webSocket.isConnected()) {
        webSocket.sendBIN((uint8_t *)sampleBuffer, bytes_read);
      }
    }

    if (bytes_read <= 0) {
      ei_printf("Error in I2S read : %d", bytes_read);
    } else {
      if (bytes_read < i2s_bytes_to_read) {
        ei_printf("Partial I2S read");
      }

      // scale the data (otherwise the sound is too quiet)
      for (int x = 0; x < i2s_bytes_to_read / 2; x++) {
        sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
      }

      if (record_status) {
        audio_inference_callback(i2s_bytes_to_read);
      } else {
        break;
      }
    }
  }
  vTaskDelete(NULL);
}

static void audio_inference_callback(uint32_t n_bytes) {
  for (int i = 0; i < n_bytes >> 1; i++) {
    inference.buffer[inference.buf_count++] = sampleBuffer[i];

    if (inference.buf_count >= inference.n_samples) {
      inference.buf_count = 0;
      inference.buf_ready = 1;
    }
  }
}

static int i2s_deinit(void) {
  i2s_driver_uninstall(MIC_i2s_num);  //stop & destroy i2s driver
  return 0;
}
#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
