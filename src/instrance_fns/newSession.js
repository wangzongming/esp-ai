
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

const log = require("../utils/log");
const createSessionId = require("../utils/createSessionId");
/***
* 启动新会话
* 一般配合 .stop 使用，用于重启一个会话
*/
function newSession(device_id) {
    !device_id && log.error(`调用 newSession 方法时，请传入 device_id`);
    if (!G_devices.get(device_id)) return;
    return new Promise((resolve) => {
        const { ws } = G_devices.get(device_id);
        const session_id = `${createSessionId()}`;
        log.t_info("session ID: " + session_id)
        ws.send(JSON.stringify({ type: "session_start", session_id }));

        // 初始化用户会话数据
        if (!G_devices.get(device_id)) return;
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            session_id,
            first_session: false,
            iat_server_connected: false,
            tts_list: new Map(),
            await_out_tts: [],
            await_out_tts_ing: false,
            await_out_tts_run: async () => {
                if (!G_devices.get(device_id)) return;
                const { await_out_tts_ing } = G_devices.get(device_id);
                if (await_out_tts_ing) return;
                async function doTask() {
                    if (!G_devices.get(device_id)) return;
                    const { await_out_tts } = G_devices.get(device_id);
                    if (await_out_tts[0]) {
                        await await_out_tts[0]();
                        await_out_tts.shift()
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            await_out_tts,
                        })
                        await doTask();
                        return;
                    } else {
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            await_out_tts_ing: false,
                        })
                    }
                }
                await doTask();
            },
            // prev_play_audio_ing: false, // 记录上次是否正在播放音频
            play_audio_ing: null,
            start_audio_time: null,
            play_audio_on_end: null,
            play_audio_seek: 0,
            stoped: false,
        })
        resolve(session_id);
    })
}
module.exports = newSession;