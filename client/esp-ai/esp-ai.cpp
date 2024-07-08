#include "esp-ai.h"

// 大多数麦克风可能默认为左通道，但可能需要将L/R引脚绑低
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define MIC_i2s_num I2S_NUM_1
#define YSQ_i2s_num I2S_NUM_0

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
// 当前电位器值
int cur_ctrl_val = 0;

// 麦克风默认配置 { bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_mic default_i2s_config_mic = {4, 5, 6};
// 扬声器默认配置 { bck_io_num, ws_io_num, data_in_num }
ESP_AI_i2s_config_speaker default_i2s_config_speaker = {16, 17, 15};
// 默认离线唤醒方案
ESP_AI_wake_up_config default_wake_up_config = {"edge_impulse", 0.7};
// { wifi 账号， wifi 密码 }
ESP_AI_wifi_config default_wifi_config = {"oldwang", "oldwang520"};
// { ip， port }
ESP_AI_server_config default_server_config = {"192.168.1.5", 8080};
// 音量配置 { 输入引脚，输入最大值，默认音量 }
ESP_AI_volume_config default_volume_config = {34, 4096, 0.5};

/** 音频缓冲区，指针和选择器 */
typedef struct
{
    int16_t *buffer;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;
static inference_t inference;
static const uint32_t sample_buffer_size = 1024; // 2048 4096
static signed short sampleBuffer[sample_buffer_size];
// 设置为true以查看从原始信号生成的特征
static bool debug_nn = false;
static bool record_status = true;
 
ESP_AI::ESP_AI() : i2s_config_mic(default_i2s_config_mic), i2s_config_speaker(default_i2s_config_speaker), wifi_config(default_wifi_config), server_config(default_server_config), wake_up_config(default_wake_up_config), volume_config(default_volume_config), debug(false)
{
}

int ESP_AI::mic_i2s_init(uint32_t sampling_rate)
{
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
        .fixed_mclk = 0};

    i2s_pin_config_t i2s_mic_pins = {
        .bck_io_num = i2s_config_mic.bck_io_num,
        .ws_io_num = i2s_config_mic.ws_io_num,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = i2s_config_mic.data_in_num,
    };

    esp_err_t ret = 0;
    ret = i2s_driver_install(MIC_i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(MIC_i2s_num, &i2s_mic_pins);

    if (ret != ESP_OK)
    {
        ei_printf("Error in i2s_driver_install");
    }

    if (ret != ESP_OK)
    {
        ei_printf("Error in i2s_set_pin");
    }

    ret = i2s_zero_dma_buffer(MIC_i2s_num);
    if (ret != ESP_OK)
    {
        ei_printf("Error in initializing dma buffer with 0");
    }

    return int(ret);
}

I2SStream i2s;
void ESP_AI::speaker_i2s_setup()
{
    AudioLogger::instance().begin(Serial, AudioLogger::Info);

    // 配置项文档
    // https://pschatzmann.github.io/arduino-audio-tools/classaudio__tools_1_1_i2_s_config_e_s_p32.html
    // esp32 代码配置处，其他的板子自行查询
    // https://github.com/pschatzmann/arduino-audio-tools/blob/9045503daae3b21300ee7bb76c4ad95efe9e1e6c/src/AudioI2S/I2SESP32.h#L186
    auto config = i2s.defaultConfig(TX_MODE);
    config.sample_rate = 16000;
    config.bits_per_sample = 16;
    config.port_no = YSQ_i2s_num; // 这里别和麦克风冲突了，esp32 有两个可用通道
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.i2s_format = I2S_MSB_FORMAT; // 这里最好看看源码中的类型定义
    config.buffer_count = 8;
    config.buffer_size = 1024;
    config.auto_clear = true;
    config.channels = 1;
    config.pin_ws = i2s_config_speaker.ws_io_num;     // LCK
    config.pin_bck = i2s_config_speaker.bck_io_num;   // BCK
    config.pin_data = i2s_config_speaker.data_in_num; // DIN
    i2s.begin(config);
}

void ESP_AI::begin(ESP_AI_CONFIG config)
{
    if (config.i2s_config_mic.bck_io_num)
    {
        i2s_config_mic = config.i2s_config_mic;
    }
    if (config.i2s_config_speaker.bck_io_num)
    {
        i2s_config_speaker = config.i2s_config_speaker;
    }
    if (strcmp(config.wifi_config.wifi_name, "") != 0)
    {
        wifi_config = config.wifi_config;
    }
    if (strcmp(config.server_config.ip, "") != 0)
    {
        server_config = config.server_config;
    }
    if (config.volume_config.input_pin)
    {
        volume_config = config.volume_config;
    }
    if (config.debug)
    {
        debug = config.debug;
    }

    if (strcmp(config.wake_up_config.wake_up_scheme, "") != 0)
    {
        wake_up_config = config.wake_up_config;
    }

    // led 指示灯
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    debug &&Serial.println("==================== WIFI ====================");
    debug &&Serial.println("wifi name: " + String(wifi_config.wifi_name));
    debug &&Serial.println("wifi pwd: " + String(wifi_config.wifi_pwd));
    WiFi.begin(wifi_config.wifi_name, wifi_config.wifi_pwd);
    debug &&Serial.print("connect wifi ing..");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        debug &&Serial.print(".");
    }

    // 不休眠，不然可能播放不了
    WiFi.setSleep(false);

    debug &&Serial.println();
    debug &&Serial.print("IP address: ");
    debug &&Serial.println(WiFi.localIP());
    debug &&Serial.println("===============================================");

    if (wake_up_config.wake_up_scheme == "edge_impulse")
    {
        debug &&Serial.println("=================== Edge Impulse ================");
        // 关键词模型相关信息和设置
        if (debug)
        {
            ei_printf("Inferencing settings:\n");
            ei_printf("\tInterval: ");
            ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
            ei_printf(" ms.\n");
            ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
            ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
            ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));
            ei_printf("2秒后开始连续推理…");
        }

        ei_sleep(2000);

        if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false)
        {
            ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
            return;
        }
        debug &&Serial.println("===============================================");
    }
    // 启动扬声器
    speaker_i2s_setup();
    // ws 服务启动
    webSocket.begin(server_config.ip, server_config.port, "/");
 
    webSocket.onEvent(std::bind(&ESP_AI::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
 
}

int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

    return 0;
}

void ESP_AI::microphone_inference_end(void)
{
    i2s_deinit();
    ei_free(inference.buffer);
}

/**
 * 返回是否连接WiFi
 */
bool ESP_AI::wifiIsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

/**
 * 返回本地IP
 * 调用案例： Serial.println(ESP_AI.localIP().c_str());
 */
std::string ESP_AI::localIP()
{
    String ipStr = WiFi.localIP().toString();
    std::string ip = std::string(ipStr.c_str());
    return std::string(ip);
}

/**
 * 记录指令回调函数
 */
void ESP_AI::onEvent(void (*func)(char command_id, char data))
{
    onEventCb = func;
}

void ESP_AI::loop()
{
    // Serial.println("loop 执行");
    webSocket.loop();

    int _cur_ctrl_val = analogRead(volume_config.input_pin);
    if (_cur_ctrl_val != cur_ctrl_val)
    {
        cur_ctrl_val = _cur_ctrl_val;
        // 计算最新音量
        volume_config.volume = static_cast<float>(cur_ctrl_val) / volume_config.max_val;
        debug &&Serial.println(volume_config.volume);
    }

    if (wake_up_config.wake_up_scheme == "edge_impulse")
    {
        // buffer 准备好后就进行推理
        if (inference.buf_ready != 0 && can_voice == "1")
        {
            // 重置 buf 状态
            inference.buf_ready = 0;
            signal_t signal;
            signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
            signal.get_data = &microphone_audio_signal_get_data;
            ei_impulse_result_t result = {0};

            EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
            if (r != EI_IMPULSE_OK)
            {
                ei_printf("ERR: Failed to run classifier (%d)\n", r);
                return;
            }
            float xmtx_val = result.classification[2].value;
            if (xmtx_val >= wake_up_config.threshold)
            {
                if (debug)
                {
                    Serial.println("");
                    Serial.print("小明同学：");
                    Serial.print(xmtx_val);
                    Serial.println("");
                }

                // 开始录音
                digitalWrite(LED_BUILTIN, HIGH);
                start_ed = "1";
                webSocket.sendTXT("start");
                debug &&Serial.println("开始录音");
            }
        }
    }

    // Complete or not
    long cur_time = millis();
    if (is_send_server_audio_over == "0" && (cur_time - last_get_audio_time > audio_delay))
    {
        is_send_server_audio_over = "1";
        digitalWrite(LED_BUILTIN, LOW);
        webSocket.sendTXT("client_out_audio_over");
    }
}

void ESP_AI::setVolume(int volume)
{
    volume_config.volume = volume;
}

void ESP_AI::wakeUp()
{
    // 开始录音
    digitalWrite(LED_BUILTIN, HIGH);
    start_ed = "1";
    webSocket.sendTXT("start");
    debug &&Serial.println("开始录音");
}

// 调整音量
void ESP_AI::adjustVolume(int16_t *buffer, size_t length, float volume)
{
    for (size_t i = 0; i < length / 2; i++)
    { // 每次处理两个字节
        buffer[i] = (int16_t)(buffer[i] * volume);
    }
}

void ESP_AI::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
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
        // 一边用喇叭，一边采集音频会很卡, 待优化...
        if (strcmp((char *)payload, "start_voice") == 0)
        {
            can_voice = "1";
            debug &&Serial.print("继续采集音频");
        }
        if (strcmp((char *)payload, "pause_voice") == 0)
        {
            can_voice = "0";
            debug &&Serial.print("暂停采集音频");
        }
        if (strcmp((char *)payload, "iat_end") == 0)
        {
            debug &&Serial.println("当前语音识别完毕啦！");
        }
        if (strcmp((char *)payload, "session_end") == 0)
        {
            start_ed = "0";
            can_voice = "1";
            digitalWrite(LED_BUILTIN, LOW);
            debug &&Serial.println("会话结束");
        }
        // 调用指令回调
        if (onEventCb != nullptr)
        {
            char *char_payload = (char *)payload;
            onEventCb(*char_payload, NULL);
            // onEventCb("open", "0001");
        }

        Serial.printf("Received Text: %s\n", payload);
        break;
    case WStype_BIN:
    {
        last_get_audio_time = millis();
        if (is_send_server_audio_over != "0")
        {
            is_send_server_audio_over = "0";
        }
        // 调整音量
        adjustVolume((int16_t *)payload, length, volume_config.volume);
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

bool ESP_AI::microphone_inference_start(uint32_t n_samples)
{
    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

    if (inference.buffer == NULL)
    {
        return false;
    }

    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    if (mic_i2s_init(EI_CLASSIFIER_FREQUENCY))
    {
        ei_printf("Failed to start I2S!");
    }

    ei_sleep(100);

    record_status = true;

    // xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void *)sample_buffer_size, 10, NULL);
    xTaskCreate(capture_samples_wrapper, "CaptureSamples", 1024 * 32, (void *)sample_buffer_size, 10, NULL);

    return true;
}

bool microphone_inference_record(void)
{
    bool ret = true;

    while (inference.buf_ready == 0)
    {
        delay(10);
    }

    inference.buf_ready = 0;
    return ret;
}

void ESP_AI::capture_samples_wrapper(void* arg) {
    ESP_AI* instance = static_cast<ESP_AI*>(arg);
    instance->capture_samples(arg);
}
void ESP_AI::capture_samples(void *arg) 
{

    const int32_t i2s_bytes_to_read = (uint32_t)arg;
    size_t bytes_read = i2s_bytes_to_read;

    while (record_status)
    {
        i2s_read(MIC_i2s_num, (void *)sampleBuffer, i2s_bytes_to_read, &bytes_read, portMAX_DELAY);
        // 发送给服务端
        if (start_ed == "1" && can_voice == "1")
        {
            if (webSocket.isConnected())
            {
                webSocket.sendBIN((uint8_t *)sampleBuffer, bytes_read);
            }
        }

        if (bytes_read <= 0)
        {
            ei_printf("Error in I2S read : %d", bytes_read);
        }
        else
        {
            if (bytes_read < i2s_bytes_to_read)
            {
                ei_printf("Partial I2S read");
            }

            // scale the data (otherwise the sound is too quiet)
            for (int x = 0; x < i2s_bytes_to_read / 2; x++)
            {
                sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
            }

            if (record_status)
            {
                audio_inference_callback(i2s_bytes_to_read);
            }
            else
            {
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

void ESP_AI::audio_inference_callback(uint32_t n_bytes)
{
    for (int i = 0; i < n_bytes >> 1; i++)
    {
        inference.buffer[inference.buf_count++] = sampleBuffer[i];

        if (inference.buf_count >= inference.n_samples)
        {
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}

int ESP_AI::i2s_deinit(void)
{
    i2s_driver_uninstall(MIC_i2s_num); // stop & destroy i2s driver
    return 0;
}
#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif