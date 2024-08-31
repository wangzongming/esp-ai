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

const log = require("../../utils/log");
/**
 * 客户端连接成功
*/
async function fn({ device_id }) {
    try {

        const { devLog, gen_client_config } = G_config;
        const { ws, client_params, client_version } = G_devices.get(device_id); 
        const user_config = await gen_client_config({ client_params, ws }) || {};
        if (!user_config.iat_server || !user_config.iat_config) {
            return log.error(`请配置 iat_server、iat_config 参数。`);
        }
        if (!user_config.llm_server || !user_config.llm_config) {
            return log.error(`请配置 llm_server、llm_config 参数。`);
        }
        if (!user_config.tts_server || !user_config.tts_config) {
            return log.error(`请配置 tts_server、tts_config 参数。`)
        }
        devLog && log.info(`---------------------------------------------------`);
        devLog && log.t_info(`客户端连接成功：${device_id}`);
        devLog && log.t_info(`客户端版本号：v${client_version}`);
        devLog && log.t_info(`用户配置 iat_server：${user_config.iat_server}`);
        devLog && log.t_info(`用户配置 iat_config：${JSON.stringify(user_config.iat_config)}`);
        devLog && log.t_info(`用户配置 llm_server：${user_config.llm_server}`);
        devLog && log.t_info(`用户配置 llm_config：${JSON.stringify(user_config.llm_config)}`);
        devLog && log.t_info(`用户配置 tts_server：${user_config.tts_server}`);
        devLog && log.t_info(`用户配置 tts_config：${JSON.stringify(user_config.tts_config)}`);
        devLog && log.info(`---------------------------------------------------`);
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            user_config: {
                f_reply: "您好",
                sleep_reply: "我先退下了，有需要再叫我。",
                connected_reply: "后台服务连接成功",
                ...user_config,
            },
        }) 

        const TTS_FN = require(`../tts`);
        const { user_config: { connected_reply } } = G_devices.get(device_id);

        ws && ws.send(JSON.stringify({ type: "stc_time", stc_time: +new Date() + "" })); 
        
        // 播放ws连接成功语音
        connected_reply && TTS_FN(device_id, {
            // text: connected_reply || `后台服务连接成功`,
            text: connected_reply,
            reRecord: false,
            pauseInputAudio: true,
            onAudioOutOver: () => {
                ws && ws.send("session_end");
            }
        })
    } catch (err) {
        console.log(err);
        log.error(`play_audio_ws_conntceed 消息错误： ${err}`)
    }

}

module.exports = fn