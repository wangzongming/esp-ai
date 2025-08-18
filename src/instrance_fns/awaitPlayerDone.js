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

function awaitPlayerDone(device_id) {
    try {
        return new Promise((resolve) => {
            if (!G_devices.get(device_id)) return resolve();
            let { awaitSpeakingTimer } = G_devices.get(device_id);
            const isSpeaking = G_Instance.isSpeaking(device_id);
            if (!isSpeaking) {
                resolve()
            } else {
                // 兜底
                let timerOutNum = 1200;
                clearInterval(awaitSpeakingTimer);
                awaitSpeakingTimer = setInterval(() => {
                    // console.log('isSpeaking::: ', isSpeaking);
                    if (!G_devices.get(device_id) || timerOutNum <= 0) {
                        // 设备断开后自动清除定时器
                        clearInterval(awaitSpeakingTimer);
                        resolve();
                        return;
                    }
                    const isSpeaking = G_Instance.isSpeaking(device_id);
                    if (!isSpeaking) {
                        clearInterval(awaitSpeakingTimer);
                        resolve();
                    }
                    timerOutNum--;
                }, 300);
                G_devices.set(device_id, { ...G_devices.get(device_id), awaitSpeakingTimer })
            }
        });

    } catch (err) {
        console.log(err);
        return false;
    }
}

module.exports = awaitPlayerDone;