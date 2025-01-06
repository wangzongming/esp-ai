
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

/***
 * 终止会话
 * 一般配合 .newSession 使用，用于重启一个会话，单独使用时就为停止会话
*/

function stop(device_id, at) {
    !device_id && log.error(`调用 stop 方法时，请传入 device_id`);
    // 清空该设备的所有任务
    if (!G_devices.get(device_id)) return;
    return new Promise((resolve) => {
        const { devLog } = G_config;
        const {
            ws,
            tts_buffer_chunk_queue, iat_server_connect_ing, iat_server_connected, client_out_audio_ing,
            tts_list = [], iat_ws, llm_ws,
            clear_audio_out_over_queue, play_audio_ing, start_audio_time, play_audio_on_end, play_audio_seek,
            iat_end_frame_timer,
            tts_server_connect_ing, tts_server_connected,
            llm_server_connect_ing, llm_server_connected
        } = G_devices.get(device_id);
        // console.log("iat_server_connect_ing", iat_server_connected)
        // console.log("tts_server_connect_ing", tts_server_connected)
        // console.log("llm_server_connect_ing", llm_server_connected) 
        if (
            iat_server_connect_ing || iat_server_connected ||
            tts_server_connect_ing || tts_server_connected ||
            llm_server_connect_ing || llm_server_connected ||
            client_out_audio_ing || play_audio_ing
        ) {
            devLog && log.t_info("打断会话");
            ws && ws.send(JSON.stringify({ type: "session_stop" }));
            try {
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    started: false, // 必须关闭，否则音频流上传时会不断创建定时器
                    stoped: true,
                    first_session: true,

                    // 清空音频播放信息
                    play_audio_ing: null,
                    prev_play_audio_ing: play_audio_ing ? true : false, // 记录上次是否正在播放音频
                    start_audio_time: null,
                    play_audio_on_end: null,
                    play_audio_seek: 0,
                })

                clearTimeout(iat_end_frame_timer);
                tts_buffer_chunk_queue.clear();
                const end_time = Date.now(); // 结束时间
                const play_time = end_time - start_audio_time; // 播放时间 
                play_audio_ing && play_audio_on_end && play_audio_on_end({
                    start_time: start_audio_time,
                    end_time: end_time,
                    play_time: Math.floor(play_time / 1000),
                    break_second: Math.floor(play_audio_seek + play_time / 1000),
                    event: "ws_disconnect",
                    seek: play_audio_seek
                });
                iat_ws && iat_ws.close && iat_ws.close()
                llm_ws && llm_ws.close && llm_ws.close()
            } catch (err) {
                console.log(err);
                log.error(`[${device_id}] ${at} 会话打断失败`);
                resolve(true);
            }

            // 清空正在播放的 tts 任务 
            for (const [key, ttsWS] of tts_list) {
                try {
                    ttsWS && ttsWS.close && ttsWS.close();
                } catch (err) {
                    console.log(err);
                    log.error(`[${device_id}] ${at} TTS 队列关闭失败`);
                }
                tts_list.delete(key)
            }
 
            resolve(true);
        } else {
            resolve(true);
        }
    });

}
module.exports = stop;