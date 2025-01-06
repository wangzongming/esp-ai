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

/**
 * 开始会话
*/
const { t_info, error } = require("../../utils/log");
const isOutTimeErr = require("../../utils/isOutTimeErr");

async function fn({ device_id }) {
    try {
        const IAT_FN = require(`../iat`);
        const TTS_FN = require(`../tts`);

        if (!G_devices.get(device_id)) {
            error(`[${device_id}] start 消息错误： 设备未连接, 将忽略本次唤醒。`);
            return;
        };

        const { auth } = G_config;
        const {
            ws,  
            client_params, 
        } = G_devices.get(device_id);
        if (auth) {
            const { success: auth_success, message: auth_message, code: auth_code } = await auth({
                ws,
                client_params: client_params,
                type: "start_session",
                send_error_to_client: (code, message) => {
                    ws.send(JSON.stringify({
                        type: "error",
                        message: message,
                        code: code
                    }));
                }
            });
            if (!auth_success) {
                ws.send(JSON.stringify({
                    type: "auth_fail",
                    message: `${auth_message || "-"}`,
                    code: isOutTimeErr(auth_message) ? "007" : auth_code,
                }));
                // 防止大量失效用户重复请求
                setTimeout(() => {
                    ws.close();
                }, 5000)
                return;
            };
        }


        await G_Instance.stop(device_id, "打断会话时");
        await G_Instance.newSession(device_id);

        const start_iat = (connect_cb) => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                started: true,
            })
 
            return IAT_FN(device_id, connect_cb);
        };
        if (!G_devices.get(device_id)) return;
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            started: true,
            start_iat,
        })
 
        
        // 应该直接去连接 iat 服务
        start_iat();

    } catch (err) {
        console.log(err);
        error(`[${device_id}] start 消息错误： ${err}`)
    }

}

module.exports = fn