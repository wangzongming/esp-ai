#if 1
#include <sys/time.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"

#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"
#include "esp_http_client.h"
#include "baidu_access_token.h"

#include "audio_idf_version.h"

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
#include "esp_netif.h"
#else
#include "tcpip_adapter.h"
#endif
extern void zx_app_main(void);

static const char *TAG = "BAIDU_SPEECH_EXAMPLE";
#define BAIDU_TTS_ENDPOINT "http://tsn.baidu.com/text2audio"
#define TTS_TEXT "今天是2025年3月26日，无锡天气晴。"

void  audio_event_task(void);
static char *baidu_access_token = NULL;
static char request_data[1024];
static audio_event_iface_handle_t evt;
static audio_pipeline_handle_t pipelineTTS;
static esp_periph_set_handle_t set;
static audio_element_handle_t http_stream_reader, i2s_stream_writer, mp3_decoder;
//该函数通过百度 OAuth2 接口获取 access_token
int _http_stream_event_handle(http_stream_event_msg_t *msg)
{
    esp_http_client_handle_t http_client = (esp_http_client_handle_t)msg->http_client;//
    if (msg->event_id != HTTP_STREAM_PRE_REQUEST) {//代表 HTTP 事件类型   （预请求）发生在 HTTP 请求发送前
        return ESP_OK;
    }
    ESP_LOGI(TAG, "【W】【%d】进入流回调函数",msg->event_id);
    if (baidu_access_token == NULL) {
        baidu_access_token = baidu_get_access_token(CONFIG_BAIDU_ACCESS_KEY, CONFIG_BAIDU_SECRET_KEY);
    }
    if (baidu_access_token == NULL) {
        ESP_LOGE(TAG, "Error issuing access token");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "【W】流回调函数字符拼接");
    int data_len = snprintf(request_data, 1024, "lan=zh&cuid=ESP32&ctp=1&tok=%s&tex=%s", baidu_access_token, TTS_TEXT);
    esp_http_client_set_post_field(http_client, request_data, data_len);//POST请求数据的函数
    esp_http_client_set_method(http_client, HTTP_METHOD_POST);
    return ESP_OK;
}

void TtsBoardInit(void)
{
// 初始化 NVS 存储
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
// 网络初始化
    ESP_ERROR_CHECK(esp_netif_init());

    // 初始化 Wi-Fi 外设集合
    esp_periph_config_t wifi_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t wifi_set = esp_periph_set_init(&wifi_cfg);

    periph_wifi_cfg_t wifi_cfg_data = {
        .wifi_config.sta.ssid = CONFIG_WIFI_SSID,
        .wifi_config.sta.password = CONFIG_WIFI_PASSWORD,
    };
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg_data);
    esp_periph_start(wifi_set, wifi_handle);
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();//生成默认的外设集合配置。
    set = esp_periph_set_init(&periph_cfg);//使用默认配置初始化外设集合，返回集合句柄 set。这个集合将用于管理 Wi-Fi、按键、SD 卡等外设。
    ESP_LOGI(TAG, "[ 1 ] Start audio codec chip");
    audio_board_handle_t board_handle = audio_board_init();  // 初始化音频板
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ESP_LOGI(TAG, "初始化音频管道");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipelineTTS = audio_pipeline_init(&pipeline_cfg);// 初始化音频管道
    mem_assert(pipelineTTS);// 确保初始化成功

    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT(); // 注册事件回调
    http_cfg.event_handle = _http_stream_event_handle;// 设置为音频流读取器
    http_cfg.type = AUDIO_STREAM_READER;//设置为音频读取流
    http_stream_reader = http_stream_init(&http_cfg);// 初始化 HTTP 流
// I2S 音频输出流
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_cfg.std_cfg.slot_cfg.slot_mode = I2S_SLOT_MODE_MONO;// 单声道
    i2s_cfg.std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT; // 左声道
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[2.3] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);  // 初始化 MP3 解码器
// 注册与链接音频元素
    ESP_LOGI(TAG, "[2.4] Register all elements to audio pipelineTTS");
    audio_pipeline_register(pipelineTTS, http_stream_reader, "http");
    audio_pipeline_register(pipelineTTS, mp3_decoder,        "mp3");
    audio_pipeline_register(pipelineTTS, i2s_stream_writer,  "i2s");
//HTTP流 → MP3解码器 → I2S流 → 编解码器（播放）
    const char *link_tag[3] = {"http", "mp3", "i2s"};
    audio_pipeline_link(pipelineTTS, &link_tag[0], 3);
//设置音频流 URI 
    audio_element_set_uri(http_stream_reader, BAIDU_TTS_ENDPOINT);
//启动事件监听器
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();// 初始化事件接口
    evt = audio_event_iface_init(&evt_cfg);// 管道监听器 音频事件接口的默认配置
    audio_pipeline_set_listener(pipelineTTS, evt);// 外设监听器
  //  audio_pipeline_run(pipelineTTS);// 启动音频管道
}

extern audio_pipeline_handle_t pipelineRecoder;
void zx_app_main(void)
{
    // 停止并清理 TTS 流水线
    if(pipelineRecoder != NULL) {
        audio_pipeline_stop(pipelineRecoder);  // 停止 TTS 播报
    }

    // 每次播放前，先停止并清理流水线
    if (pipelineTTS != NULL) {
        ESP_LOGI(TAG, "停止并清理 TTS 流水线");

        // 停止流水线并等待完全停止
        audio_pipeline_stop(pipelineTTS);
        audio_pipeline_wait_for_stop(pipelineTTS);  // 等待完全停止

        // 重置流水线的缓存区和元素状态
        audio_pipeline_reset_ringbuffer(pipelineTTS);
        audio_pipeline_reset_elements(pipelineTTS);
        audio_pipeline_reset_items_state(pipelineTTS);

        // 增加短暂延时，确保状态完全同步
        vTaskDelay(pdMS_TO_TICKS(100));  // 等待100ms，确保状态完全同步
    }

    // 清空并重新设置 URI
    audio_element_set_uri(http_stream_reader, NULL);  // 清空 URI
    audio_element_set_uri(http_stream_reader, BAIDU_TTS_ENDPOINT);  // 重新设置 URI

    // 重新运行 TTS 流水线
    if (pipelineTTS != NULL) {
        ESP_LOGI("W", "管道流开启");
        esp_err_t ret = audio_pipeline_run(pipelineTTS);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "启动 TTS 流水线失败, 错误码: %d", ret);
            // 如果失败，可以添加重试机制
            vTaskDelay(pdMS_TO_TICKS(500));  // 等待500ms后重试
            ret = audio_pipeline_run(pipelineTTS);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "重试 TTS 流水线启动失败, 错误码: %d", ret);
            }
        }
    }

    audio_event_task();  // 启动音频事件任务
}


void audio_event_task(void)
{
    audio_event_iface_msg_t msg;
    esp_err_t ret;
    while (1) {
        // 等待音频事件
        ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        ESP_LOGI(TAG, "【W】【%d】音频事件发生", ret);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error: %d", ret);
            continue;
        }

        // 处理 MP3 解码器的音乐信息事件
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) mp3_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(mp3_decoder, &music_info);
            ESP_LOGI(TAG, "动态设置 I2S 时钟, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);
            // 动态设置 I2S 时钟
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;
        }

        // 处理 I2S 播放器的停止事件
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "处理 I2S 播放器的停止事件");

            // 停止流水线
            audio_pipeline_stop(pipelineTTS);
            audio_pipeline_wait_for_stop(pipelineTTS);  // 等待完全停止

            // 检查流水线状态
            if (pipelineRecoder != NULL) {
                ESP_LOGI(TAG, "恢复录音");
                audio_pipeline_run(pipelineRecoder);  // 启动录音
            }
            break;
        }
    }
#if 0
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    audio_pipeline_unregister(pipeline, http_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, mp3_decoder);
// !~
    audio_pipeline_remove_listener(pipeline);
    audio_event_iface_destroy(evt);
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(http_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(mp3_decoder);
#endif
}
/*
✅ 整体功能流程总结
    停止音频管道
        停止管道上的所有任务。
        等待所有任务完全停止。
        强制终止管道（确保彻底停止）。
    取消注册
        将所有音频元素与管道解绑。
    移除监听器
        清理监听器，防止再接收事件。
    停止外设
        停止所有外设任务。
        移除外设监听器。
    释放资源
        销毁事件接口。
        释放音频管道、音频元素和外设集合的内存，防止内存泄漏。
*/

#else

#endif