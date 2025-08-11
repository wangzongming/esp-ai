#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_websocket_client.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "mp3_decoder.h"
#include "board.h"
#include "esp_audio.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "input_key_service.h"
#include "periph_adc_button.h"
#include "board.h"

#include "sdcard_list.h"
#include "sdcard_scan.h"

#include "cJSON.h" 
#include "websocket_rx_handler.h"

#if 1
static const char *TAG = "websocket_demo";


#define WIFI_SSID       "901"
#define WIFI_PASS       "102118abc"
#define WEBSOCKET_URI   "ws://192.168.10.2:8088/?v=1.0.0&device_id=94:BB:43:ED:67:98"
extern QueueHandle_t json_msg_queue;
static audio_element_handle_t raw_tx_stream = NULL;
static audio_element_handle_t raw_rx_stream = NULL;
esp_websocket_client_handle_t client = NULL;

static void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
}
#define PCM_FILE_PATH "/sdcard/recorded_audio.pcm"
#define BUFFER_SIZE   1024
#define RECORD_SECONDS 10  // 录音时长，可改

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket 连接成功");

             char message[64];
            snprintf(message, sizeof(message), "{\"type\":\"play_audio_ws_conntceed\"}");
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            break;
            

        case WEBSOCKET_EVENT_DISCONNECTED:
                
            break;
           #if 0
            if (data->data_len > 0) {
             char *payload = strndup((const char *)data->data_ptr, data->data_len);
           if (payload) {
            ESP_LOGI(TAG, "WebSocket Payload: %s", payload);

            ws_msg_info_t msg_info;
            if (parse_ws_message(payload, &msg_info)) {
                ESP_LOGI(TAG, "解析成功，type = %s", msg_info.type);
                for (int i = 0; i < msg_info.field_count; i++) {
                    ESP_LOGI(TAG, "字段 %s = %s", msg_info.fields[i].key, msg_info.fields[i].value);
                }
                // TODO: 按类型处理，例如:
                if (strcmp(msg_info.type, "play_audio") == 0) {
                    // 使用字段 msg_info.fields[i] 等数据处理
                }
                } else {
                        // 写入到音频流
                        ESP_LOGI(TAG, "WebSocket received stream %d bytes", data->data_len);
                        raw_stream_write(raw_rx_stream, (char *)data->data_ptr, data->data_len);
                    ESP_LOGW(TAG, "收到无效 JSON 或不含 type 字段");
                }
                free(payload);
                }
            }
            #endif
        case WEBSOCKET_EVENT_DATA:
             ESP_LOGI(TAG, "WebSocket 接收, op_code=%d data_len=%d", data->op_code, data->data_len);
             if(data->op_code == 1)
             {
                    if (data->data_len > 0 && json_msg_queue != NULL) {
                        char *recv_buf = malloc(data->data_len + 1);
                        if (recv_buf) {
                            memcpy(recv_buf, data->data_ptr, data->data_len);
                            recv_buf[data->data_len] = '\0';

                            if (xQueueSend(json_msg_queue, &recv_buf, 0) != pdPASS) {
                                ESP_LOGW(TAG, "json_msg_queue 满了，释放 recv_buf");
                                free(recv_buf);
                            }
                        } else {
                            ESP_LOGE(TAG, "recv_buf malloc 失败");
                        }
                    }
            }
            else if(data->op_code == 2)
            {
                    if (data->data_len >= 6) {
                    char session_id_string[5] = {0}; // C 字符串，自动 '\0' 结尾
                    memcpy(session_id_string, data->data_ptr, 4);
                    // 直接打印或用 char[] 继续处理
                    ESP_LOGI("SESSION", "Session ID: %s", session_id_string);
                    char session_status_string[3] = {0};
                    memcpy(session_status_string, data->data_ptr + 4, 2);
                    ESP_LOGI("SESSION", "Session Status: %s", session_status_string);
                    ESP_LOG_BUFFER_HEX("WEBSOCKET", data->data_ptr, 6);
                    // 写入到音频流
                    ESP_LOGI(TAG, "写入音频流 %d bytes", data->data_len);
                    raw_stream_write(raw_rx_stream, (char *)data->data_ptr + 6, data->data_len - 6);
                        
                    } else {
                        // 错误处理
                        ESP_LOGE(TAG, "Payload too short for session info");
                    }
                
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "WebSocket error");
            break;
        default:
            break;
    }
}

static void websocket_client_init(void) {
    esp_websocket_client_config_t websocket_cfg = {
        .buffer_size = 1024*20, // 设置缓冲区大小
        .uri = WEBSOCKET_URI,
        .disable_auto_reconnect = false,
        .reconnect_timeout_ms = 10000,
    };
    client = esp_websocket_client_init(&websocket_cfg);
      if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return;
    }
    ESP_ERROR_CHECK(esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, client));
    esp_websocket_client_start(client);
}
extern char Lock_SendPCM;
static void audio_capture_task(void *pvParameters) {
    uint8_t buffer[1024];
    int len = 0;

int i_n=0;
 int total_bytes = 0;
    while (1) {
        if ( esp_websocket_client_is_connected(client)) {
            if(Lock_SendPCM==1)
            {
                ESP_LOGI(TAG, "开始准备发送音频数据\\r\n");
                vTaskDelay(pdMS_TO_TICKS(500));  
                Lock_SendPCM=2;
             }
            if(Lock_SendPCM==2)
            {
              ESP_LOGI(TAG, "WebSocket sending %d bytes", len);
                len = raw_stream_read(raw_tx_stream, (char *)buffer, sizeof(buffer));
                esp_websocket_client_send_bin(client, (const char *)buffer, len, portMAX_DELAY);
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
playlist_operator_handle_t sdcard_list_handle = NULL;
void sdcard_url_save_cb(void *user_data, char *url)
{
    playlist_operator_handle_t sdcard_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_handle, url);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Fail to save sdcard url to sdcard playlist");
    }
}
void app_main(void) {
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    websocket_app_start();
    // 初始化 WiFi & WebSocket
    wifi_init();
    websocket_client_init();

    // 初始化音频外设
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    audio_board_sdcard_init(set, SD_MODE_1_LINE);

    ESP_LOGI(TAG, "[1.2] Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);
    sdcard_scan(sdcard_url_save_cb, "/sdcard", 0, (const char *[]) {"mp3"}, 1, sdcard_list_handle);
    sdcard_list_show(sdcard_list_handle);

    audio_board_handle_t board_handle = audio_board_init();
     audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    //audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_ENCODE, AUDIO_HAL_CTRL_START);
    

    // TX: I2S -> raw -> websocket
    audio_pipeline_handle_t pipeline_tx;
    audio_element_handle_t i2s_reader;

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline_tx = audio_pipeline_init(&pipeline_cfg);

    i2s_stream_cfg_t i2s_reader_cfg = I2S_STREAM_CFG_DEFAULT_WITH_TYLE_AND_CH(
    I2S_NUM_0,        // I2S port
    16000,            // Sample rate
    16,               // Bits
    AUDIO_STREAM_READER, // 类型：输入
    1                 // 1 表示单声道
);

    i2s_reader = i2s_stream_init(&i2s_reader_cfg);
    i2s_stream_set_clk(i2s_reader, 16000, 16, 1);
    
    raw_stream_cfg_t raw_tx_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_tx_cfg.type = AUDIO_STREAM_WRITER;
    raw_tx_stream = raw_stream_init(&raw_tx_cfg);

    audio_pipeline_register(pipeline_tx, i2s_reader, "i2s_reader");
    audio_pipeline_register(pipeline_tx, raw_tx_stream, "raw_tx_stream");
    audio_pipeline_link(pipeline_tx, (const char *[]){"i2s_reader", "raw_tx_stream"}, 2);
    audio_pipeline_run(pipeline_tx);
    
    audio_element_info_t i2s_info;
    audio_element_getinfo(i2s_reader, &i2s_info);

    ESP_LOGI(TAG, "I2S Reader Sample Rate: %d", i2s_info.sample_rates);
    ESP_LOGI(TAG, "I2S Reader Bits: %d", i2s_info.bits);
    ESP_LOGI(TAG, "I2S Reader Channels: %d", i2s_info.channels);
    xTaskCreate(audio_capture_task, "audio_capture_task", 4096, NULL, 5, NULL);

    // RX: websocket -> raw -> mp3_decoder -> i2s
    audio_pipeline_handle_t pipeline_rx;
    audio_element_handle_t mp3_decoder, i2s_writer;

    audio_pipeline_cfg_t pipeline_cfg_rx = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline_rx = audio_pipeline_init(&pipeline_cfg_rx);

    raw_stream_cfg_t raw_rx_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_rx_cfg.type = AUDIO_STREAM_READER;
    raw_rx_stream = raw_stream_init(&raw_rx_cfg);

    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

      i2s_stream_cfg_t i2s_writer_cfg = I2S_STREAM_CFG_DEFAULT_WITH_TYLE_AND_CH(
        I2S_NUM_0,        // I2S port
        16000,            // Sample rate
        16,               // Bits
        AUDIO_STREAM_WRITER, // 类型：输入
        1                 // 1 表示单声道
    );
    i2s_writer = i2s_stream_init(&i2s_writer_cfg);
   // i2s_stream_set_clk(i2s_writer, 16000, 16, 1);

    audio_pipeline_register(pipeline_rx, raw_rx_stream, "raw_rx_stream");
    audio_pipeline_register(pipeline_rx, mp3_decoder, "mp3_decoder");
    audio_pipeline_register(pipeline_rx, i2s_writer, "i2s_writer");

    audio_pipeline_link(pipeline_rx, (const char *[]){"raw_rx_stream", "mp3_decoder", "i2s_writer"}, 3);
    audio_pipeline_run(pipeline_rx);

int Num = 0;
    while (1) {
        Num++;
        if(Num == 20) {
            char message[64];
            snprintf(message, sizeof(message), "{ \"type\":\"start\" }");
            ESP_LOGI(TAG, "发送开始消息: %s", message);
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif
