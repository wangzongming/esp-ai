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
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "../configs/common.h" 

struct GetPositionContext
{
    bool *debug; 
    bool (*IS_WL_CONNECTED)();
    std::function<void(const String &ip, const String &nation, const String &province, const String &city, const String &latitude, const String &longitude)> onPositionCb;
};

void get_position_task_static(void *arg);

extern StaticTask_t getPositionContextTaskBuffer;
extern StackType_t getPositionContextTaskStack[GET_POSITION_TASK_SIZE];
