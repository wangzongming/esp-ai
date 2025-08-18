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

/**
 * 让客户端输出一段话
 */
async function llm(device_id, text, opts) {
    try {
        !device_id && log.error(`调用 llm 方法时，请传入 device_id`);
        if (!G_devices.get(device_id)) return;
        // 预先打通LLM连接通道
        const LLM_FN = require(`../functions/llm`);
        LLM_FN(device_id, {is_pre_connect: true});
        // 结构客户端参数
        const {session_id: _session_id} = G_devices.get(device_id);
        // 先打断当前会话
        await G_Instance.stop(device_id, "打断会话时");
        // 生成新的会话ID
        const session_id = await G_Instance.newSession(device_id);
        if (
            _session_id &&
            session_id !== _session_id &&
            !([G_session_ids["tts_fn"]].includes(session_id))
        ) return;

        // 设置客户端参数
        if (!G_devices.get(device_id)) return;
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            start_iat: async ()=>{
                // 直接开启LLM推理，不主动开启下一轮IAT会话
                return true;
            },
        })

        // 开启LLM推理
        text && G_Instance.matchIntention(device_id, text);
        text && LLM_FN(device_id, { text, ...opts })
        return true;
    } catch (err) {
        console.log(err);
        log.error(`run_llm_ws_connected 消息错误： ${err}`);
        return false;
    }
}

module.exports = llm;
