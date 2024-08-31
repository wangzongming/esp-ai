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
const play_temp = require('../../audio_temp/play_temp')
const createSessionId = require("../../utils/createSessionId");
const { t_info, error } = require("../../utils/log");
async function fn({ device_id }) {
    try {
        const IAT_FN = require(`../iat`);
        const TTS_FN = require(`../tts`);

        const { devLog, auth } = G_config;
        const {
            ws, iat_server_connected,
            tts_list = [], iat_ws, llm_ws, client_params,
            first_session, client_out_audio_ing, iat_server_connect_ing, clear_audio_out_over_queue,
            user_config: { f_reply },
            play_audio_ing, start_audio_time, play_audio_on_end, play_audio_seek
        } = G_devices.get(device_id);
        if (auth) {
            const { success: auth_success, message: auth_message } = await auth(client_params, "start_session");
            if (!auth_success) {
                ws.send(JSON.stringify({ type: "auth_fail", message: `${auth_message || "-"}` }));
                ws.close();
                return;
            };
        }

        if (iat_server_connect_ing || iat_server_connected || client_out_audio_ing) {
            devLog && t_info("打断会话");
            try {

                const end_time = Date.now(); // 结束时间
                const play_time = end_time - start_audio_time; // 播放时间
                play_audio_ing && play_audio_on_end && play_audio_on_end({
                    start_time: start_audio_time,
                    end_time: end_time,
                    play_time: play_time / 1000,
                    break_second: play_audio_seek + play_time / 1000,
                    event: "user_break",
                    seek: play_audio_seek
                });
                iat_ws && iat_ws.close && iat_ws.close()
                llm_ws && llm_ws.close && llm_ws.close()
            } catch (err) {
                console.log(err);
                error(`会话打断失败`);
            }

            // 清空正在播放的 tts 任务 
            for (const [key, ttsWS] of tts_list) {
                try {
                    ttsWS && ttsWS.close && ttsWS.close();
                } catch (err) {
                    console.log(err);
                    error(`TTS 队列关闭失败`);
                }
                tts_list.delete(key)
            }

            // 清空 tts 回调
            clear_audio_out_over_queue();
        };

        const session_id = `${createSessionId()}`;
        t_info("session ID: " + session_id)
        ws.send(JSON.stringify({ type: "session_start", session_id }));
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            session_id,
            first_session: false,
            iat_server_connected: false,
            tts_list: new Map(),
            await_out_tts: [],
            await_out_tts_ing: false,
            await_out_tts_run: async () => {
                const { await_out_tts_ing } = G_devices.get(device_id);
                if (await_out_tts_ing) return;
                async function doTask() {
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
            play_audio_ing: null,
            start_audio_time: null,
            play_audio_on_end: null,
            play_audio_seek: 0
        })
        const start_iat = (connect_cb) => {
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                started: true,
            })

            ws && ws.send("start_voice");
            return IAT_FN(device_id, connect_cb);
        };
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            started: true,
            start_iat,
        })

        if (first_session) {
            TTS_FN(device_id, {
                text: f_reply || "您好",
                reRecord: true,
                pauseInputAudio: true
            });
        } else {
            start_iat(async () => {
                await play_temp("du.pcm", ws, 0.8, 24);
            });
        }
    } catch (err) {
        console.log(err);
        log.error(`start 消息错误： ${err}`)
    }

}

module.exports = fn