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

const { tts_info } = require("../../utils/log");
const { clear_sid } = require("../device_fns");

async function fn({ device_id, tts_task_id, session_id, session_status }) {
    const { devLog } = G_config;
    if (!G_devices.get(device_id)) return;
    const { play_audio_on_end } = G_devices.get(device_id);
    devLog && tts_info("-> 客户端音频流播放完毕-> 会话ID：" + session_id + "  会话状态：" + session_status + "  任务ID：" + tts_task_id);

    const new_attr = { client_out_audio_ing: false, };
    if (session_status === G_session_ids["tts_all_end"] || session_status === G_session_ids["tts_all_end_align"]) {
        if (session_id !== G_session_ids["tts_fn"]) { // TTS 方法不应打乱对话 
            // 如果是同一个会话的情况下为中断对话 
            clear_sid(device_id);
        }

        // 音频播放结束 
        G_devices.set(device_id, { ...G_devices.get(device_id), play_audio_ing: false });
    }

    if (session_status === G_session_ids["tts_all_end"]) {
        tts_task_id === G_session_ids["play_music"] && play_audio_on_end && play_audio_on_end("play_end");
    }

    G_devices.set(device_id, {
        ...G_devices.get(device_id),
        ...new_attr
    })
}

module.exports = fn