/**
 * Copyright (c) 2024 小明IO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Commercial use of this software requires prior written authorization from the Licensor.
 * 请注意：将 ESP-AI 代码用于商业用途需要事先获得许可方的授权。
 * 删除与修改版权属于侵权行为，请尊重作者版权，避免产生不必要的纠纷。
 * 
 * @author 小明IO   
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */
#pragma once 
#include <Arduino.h> 
#include "../configs/common.h"
 
// 前向声明模板类
// namespace audio_tools { 
//     template <typename T> class QueueStream;
//     template <typename T> class StreamCopyT;
// }

struct PlayAudioContext {  
    // 读取剩余音频缓冲区数量
    int (*available)();

    // 执行一次音频数据拷贝
    void (*copy)();

    // 上报当前 buffer 可用空间到服务端
    void (*sendTXT)(const char *msg);

    // 是否正在播放音频
    bool *spk_ing;

    // 是否连接到 websocket
    bool *esp_ai_ws_connected;

    // 会话 ID 指针
    const String *esp_ai_session_id;    
};

void play_audio_task_static(void* arg);

extern StaticTask_t playAudioTaskBuffer;
extern StackType_t playAudioTaskStack[PLAY_AUDIO_TASK_SIZE];
