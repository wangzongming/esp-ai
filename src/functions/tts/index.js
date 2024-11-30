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
// const delay = require("../../utils/delay");
const createUUID = require("../../utils/createUUID");

let index = 0;

/**
 * @param {Buffer}    is_over       是否完毕
 * @param {Buffer}    audio         音频流
 * @param {WebSocket} tts_task_id   WebSocket 连接key
 * @param {WebSocket} ws            WebSocket 连接
 * @param {Function}  resolve       TTS 函数的 resolve 参数
*/
async function cb({ device_id, is_over, audio, ws, tts_task_id, resolve, reRecord, session_id, text_is_over, need_record, frameOnTTScb, is_cache }) {
    try {
        const { devLog, onTTScb } = G_config;
        if (!G_devices.get(device_id)) return;
        const {
            ws: ws_client, tts_list, add_audio_out_over_queue, session_id: now_session_id, du
        } = G_devices.get(device_id);
        if (session_id && session_id !== now_session_id) return;


        onTTScb && onTTScb({
            device_id,
            is_over,
            audio,
            ws: ws_client,
            instance: G_Instance,
            sendToClient: () => ws_client && ws_client.send(JSON.stringify({
                type: "instruct",
                command_id: "on_tts_cb",
                data: audio.toString('base64')
            }))
        });
        frameOnTTScb && frameOnTTScb(audio, is_over);

        if (!resolve) {
            log.error('TTS 插件中，调用 cb 时 resolve 参数不能为空');
        }
        ws_client.isAlive = true;

        const send_end_flag = () => {
            if (text_is_over) {
                const sid = need_record ? "2000" : "2001";
                devLog && log.tts_info(`-> 服务端发送 LLM 结束的标志流: ${sid}`);
                const endFlagBuf = Buffer.from(sid, 'utf-8');
                ws_client.send(endFlagBuf);

                ws_client && ws_client.send(JSON.stringify({
                    type: "session_status",
                    status: "tts_real_end",
                }));
            }
        }
        if(is_cache) {
            send_end_flag();
            return;
        };
        /**
         * 1. TTS 转换完毕，并且发往客户端
         * 2. 客户端告知服务已经完成音频流播放
         * 3. 本任务完成
        */
        if (is_over) {
            devLog && log.tts_info('-> TTS 转换完毕: ', audio.length);
            ws.close && ws.close()
            tts_list.delete(tts_task_id)

            async function overToDo() {
                if (!G_devices.get(device_id)) return;
                const { ws: ws_client, start_iat } = G_devices.get(device_id);
                if (reRecord) {
                    add_audio_out_over_queue("warning_tone", () => {
                        start_iat && start_iat();
                        resolve(true);
                    })
                    await du();
                } else {
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        stoped: true,
                    })

                    resolve(true);
                }
            }
            if (!audio.length) {
                overToDo();
                ws_client.send(JSON.stringify({ type: "tts_send_end", tts_task_id }));
                send_end_flag();
            } else {
                add_audio_out_over_queue(tts_task_id, overToDo)
            }
        }

        let c_l = G_max_audio_chunk_size;
        const alen = audio.length;
        for (let i = 0; i < audio.length; i += c_l) {
            if (!G_devices.get(device_id)) return; 
            const { session_id: now_session_id, useed_flow } = G_devices.get(device_id);
            // if(stoped) return;
            if (session_id && now_session_id !== session_id) {
                log.t_info("用户终止流")
                break;
            }

            const end = Math.min(i + c_l, audio.length);
            const chunk = audio.slice(i, end);
            if (!(Buffer.isBuffer(chunk))) {
                log.t_info(`跳过无效 chunk: ${i}`);
                continue;
            }

            // session_id
            const _session_id = session_id ? `${session_id}` : "0000";
            const sessionIdBuffer = Buffer.from(_session_id, 'utf-8');
            const combinedBuffer = Buffer.concat([sessionIdBuffer, chunk]);
            // console.log('chunk length', combinedBuffer.length)
            ws_client.send(combinedBuffer);

            // test... 
            // G_devices.set(device_id, {
            //     ...G_devices.get(device_id),
            //     useed_flow: useed_flow + (combinedBuffer.length / 1024)
            // })

            if (is_over && (end >= alen)) {
                ws_client.send(JSON.stringify({ type: "tts_send_end", tts_task_id }));
                send_end_flag();
            }
            index++;
        }

        // test... 
        // const { useed_flow } = G_devices.get(device_id);
        // console.log(`->>> 已发送：${useed_flow.toFixed(2)} kb`)
    } catch (err) {
        console.log(err);
        log.error(`[${device_id}] TTS 回调错误： ${err}`)
    }


}

/**
 * TTS 模块
 * @param {String} device_id 设备id 
 * @param {String} text 待播报的文本 
 * @param {Boolean} pauseInputAudio 客户端是否需要暂停音频采集
 * @param {Boolean} reRecord TTS播放完毕后是再次进入iat识别环节，服务端控制
 * @param {Boolean} session_id 会话id(这里绝不是从设备信息中取，设备信息会实时更新)
 * @param {Boolean} text_is_over 文本是否完整
 * @param {Boolean} need_record  是否需要重新识别，由客户端控制
 * @param {Boolean} frameOnTTScb 上层要进行流监听时提供的回调
 * @return {Function} (pcm)=> Promise<Boolean>
 * 
*/
module.exports = (device_id, opts) => {
    try {
        const { devLog, plugins = [], tts_params_set, onTTS } = G_config;
        const { ws: ws_client, error_catch, tts_list, add_audio_out_over_queue, user_config: { iat_server, llm_server, tts_server, tts_config } } = G_devices.get(device_id);
        const { text, pauseInputAudio, reRecord, onAudioOutOver, session_id, text_is_over = true, need_record = false, frameOnTTScb, is_cache } = opts;
        const plugin = plugins.find(item => item.name == tts_server && item.type === "TTS")?.main;

        const TTS_FN = plugin || require(`./${tts_server}`);
        if (!text || !(`${text}`.replace(/\s/g, ''))) {
            return true;
        }
        devLog && log.info("");
        devLog && log.tts_info('=== 开始请求TTS: ', text, " ===");

        // 开始播放直接先让 esp32 暂停采集音频，不然处理不过来
        if (pauseInputAudio) {
            if (!G_devices.get(device_id)) return;
            ws_client && ws_client.send("pause_voice");
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                client_out_audio_ing: true,
            })
        }

        // 任务ID
        const tts_task_id = createUUID();
        onAudioOutOver && add_audio_out_over_queue(tts_task_id, onAudioOutOver)
        onTTS && onTTS({
            device_id, tts_task_id,
            ws: ws_client,
            text,
            text_is_over,
            instance: G_Instance,
            sendToClient: () => ws_client && ws_client.send(JSON.stringify({
                type: "instruct",
                command_id: "on_tts",
                data: text
            }))
        });


        const tts_cache_key = `${device_id}_${text}`;
        const cache_TTS = G_get_cahce_TTS(tts_cache_key);
        if (cache_TTS && !is_cache) {
            console.log('使用缓存 TTS ')
            return ()=> new Promise((resolve)=>{
                cb({
                    audio: cache_TTS,
                    is_over: true,
                    tts_task_id, device_id, reRecord, session_id, text_is_over, need_record, frameOnTTScb
                })
                resolve(true);
            });
        }

        /**
         * 记录 tts 服务对象
        */
        const logWSServer = (wsServer) => {
            tts_list.set(tts_task_id, wsServer)
        }

        /**
         * 开始连接 tts 服务的回调
         */
        const connectServerBeforeCb = () => {
            if (!G_devices.get(device_id)) return;
            // console.time("TTS 服务连接时间：")
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                tts_server_connect_ing: true,
            })
        }

        /**
        * 连接 tts 服务后的回调
        */
        const connectServerCb = (connected) => {
            if (connected) {
                if (!G_devices.get(device_id)) return;
                // console.timeEnd("TTS 服务连接时间：")
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    tts_server_connected: true,
                    tts_server_connect_ing: false,
                })

                ws_client && ws_client.send(JSON.stringify({
                    type: "session_status",
                    status: "tts_chunk_start",
                }));
            } else {
                if (!G_devices.get(device_id)) return;
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    tts_server_connected: false,
                    tts_server_connect_ing: false,
                })
                ws_client && ws_client.send(JSON.stringify({
                    type: "session_status",
                    status: "tts_chunk_end",
                }));
            }
        }

        /**
         * tts 服务发生错误时调用
        */
        const ttsServerErrorCb = (err) => {
            error_catch("TTS", "302", err);
            tts_list.delete(tts_task_id)
            log.error(err)
        }

        ws_client && ws_client.send(JSON.stringify({ type: "play_audio", tts_task_id }));
        return TTS_FN({
            text,
            device_id,
            devLog,
            tts_config,
            tts_params_set,
            log,
            iat_server, llm_server, tts_server,
            cb: (arg) => cb({ ...arg, tts_task_id, device_id, reRecord, session_id, text_is_over, need_record, frameOnTTScb, is_cache }),
            logWSServer,
            ttsServerErrorCb,
            connectServerBeforeCb,
            connectServerCb,
        })
    } catch (err) {
        console.log(err);
        log.error(`TTS 错误： ${err}`)
    }

};
