set(COMPONENT_SRCDIRS  .)
# ✅ 将当前目录下所有 .c 文件加入编译
file(GLOB_RECURSE COMPONENT_SRCS "ws_echo_server.c" "https_request_example_main.c" "play_baidu_speech_mp3_example.c" "main.c" "*.c")
# ✅ 显式添加 cloud_services 头文件路径
set(COMPONENT_ADD_INCLUDEDIRS 
    .
    "D:/ESPIDF/esp32_5.2/TOOL/esp-adf/components/adf_utils/cloud_services/include"
    "D:/ESPIDF/esp32_5.2/TOOL/esp-adf/components/input_key_service/include"
    "D:/ESPIDF/esp32_5.2/TOOL/esp-adf/components/playlist/include"
)

set(COMPONENT_REQUIRES 
    esp_actions
    audio_sal
    audio_stream
    audio_hal
    audio_board
    audio_recorder
    esp_peripherals
    audio_flash_tone
)

# ✅ 额外组件路径
set(EXTRA_COMPONENT_DIRS "D:/ESPIDF/esp32_5.2/TOOL/esp-adf/components")

# 使用 IDF 的新版本宏替代旧版 `register_component()`
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES ${COMPONENT_REQUIRES}
    EMBED_TXTFILES server_root_cert.pem local_server_cert.pem
)
