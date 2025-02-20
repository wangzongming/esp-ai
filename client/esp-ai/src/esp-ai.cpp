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
 
#include "esp-ai.h"
 
#include "wakeUp/edge-impulse/main.h"
#include "init/speaker_i2s_setup.h"
#include "init/mic_i2s_init.h" 
#include "webSocketEvent/main.h"

#include "private/adjustVolume.h"

#include "sdk/begin.h"
#include "sdk/loop.h"
#include "sdk/setWifiConfig.h"
#include "sdk/wifiIsConnected.h" 
#include "sdk/localIP.h" 
#include "sdk/onEvent.h" 
#include "sdk/onError.h" 
#include "sdk/wakeUp.h" 
#include "sdk/setVolume.h"  

#include "webServer/main.h" 
 
ESP_AI::ESP_AI() : debug(false), wifi_config(default_wifi_config), server_config(default_server_config), wake_up_config(default_wake_up_config), volume_config(default_volume_config), i2s_config_mic(default_i2s_config_mic), i2s_config_speaker(default_i2s_config_speaker), reset_btn_config(default_reset_btn_config), lights_config(default_lights_config)
{
} 
