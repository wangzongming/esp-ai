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
#include <functional>
#include "../configs/common.h"
#include "../audio/zh/hui_fu_chu_chang.h"

struct OnRepeatedlyClickContext
{  
    bool *debug;
    const String *esp_ai_session_id; 
    const String *power; 
    int *pin;
    void (*play_builtin_audio)(const unsigned char *data, size_t len);         
    std::function<void()> wait_mp3_player_done;
    std::function<void()> on_repeatedly_click_cb;
    std::function<void()> clear_data;
};

void on_repeatedly_click_task_static(void *arg);

extern StaticTask_t onRepeatedlyClickTaskBuffer;
extern StackType_t onRepeatedlyClickTaskStack[ON_REPEATEDLY_CLICK_TASK_SIZE];
