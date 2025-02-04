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
const createUUID = require("../../utils/createUUID");
const Audio_sender = require("../../utils/audio_sender");

/**
 * @param {Buffer}    is_over       是否完毕
 * @param {Buffer}    audio         音频流
 * @param {WebSocket} tts_task_id   WebSocket 连接key
 * @param {WebSocket} ws            WebSocket 连接
*/
async function cb({ device_id, is_over, audio, ws, tts_task_id, session_id, text_is_over, need_record, frameOnTTScb, is_create_cache, audio_sender }) {
    try {
        const { devLog, onTTScb } = G_config;
        if (!G_devices.get(device_id)) return;
        const {
            ws: ws_client, tts_list, session_id: now_session_id
        } = G_devices.get(device_id);
        if (!is_create_cache && session_id && now_session_id && session_id !== now_session_id) return;

        !is_create_cache && onTTScb && onTTScb({
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
        !is_create_cache && frameOnTTScb && frameOnTTScb(audio, is_over);

        ws_client.isAlive = true;

        audio.length && audio_sender.sendAudio(audio);
        // 告诉客户端本 TTS chunk 播放完毕
        if (is_over) {
            devLog && log.tts_info('-> TTS 片段转换完毕，数量: ', audio.length);
            ws.close && ws.close();
            tts_list.delete(tts_task_id);

            // 这个和会话ID不同，这都是追加的方式。
            if (text_is_over) {
                audio_sender.sendAudio(Buffer.from(need_record ? G_session_ids["tts_all_end_align"] : G_session_ids["tts_all_end"], 'utf-8'));
            } else {
                audio_sender.sendAudio(Buffer.from(G_session_ids["tts_chunk_end"], 'utf-8'));
            }
        }

    } catch (err) {
        console.log(err);
        log.error(`[${device_id}] TTS 回调错误： ${err}`)
    }


}

/**
 * TTS 模块
 * @param {String} device_id 设备id
 * @param {String} text 待播报的文本 
 * @param {Boolean} session_id 会话id(这里绝不是从设备信息中取，设备信息会实时更新)
 * @param {Boolean} text_is_over 文本是否完整，或者文本是否是最后一段
 * @param {Boolean} need_record  是否需要重新识别，由客户端控制
 * @param {Boolean} frameOnTTScb 上层要进行流监听时提供的回调
 * @param {Boolean} is_create_cache 创造缓存数据二执行的函数
 * @return {Function} (pcm)=> Promise<Boolean>
 *
*/
function TTSFN(device_id, opts) {
    return new Promise((resolve) => {
        try {
            const { devLog, plugins = [], tts_params_set, onTTS } = G_config;
            const {
                ws: ws_client, error_catch, tts_list,
                user_config: { iat_server, llm_server, tts_server, tts_config }
            } = G_devices.get(device_id);
            const { text, session_id, text_is_over = true, need_record = false, frameOnTTScb, is_cache, is_create_cache, tts_task_id = createUUID() } = opts;
            const plugin = plugins.find(item => item.name === tts_server && item.type === "TTS")?.main;

            const TTS_FN = plugin || require(`./${tts_server}`);

            if (!text || !(`${text}`.replace(/\s/g, ''))) {
                return true;
            }
            if (is_create_cache) {
                devLog && log.tts_info('-> 开始缓存TTS: ', text);
            } else {
                devLog && log.tts_info('-> 开始请求TTS: ', text);
            }
            const audio_sender = new Audio_sender(ws_client, device_id)

            !is_create_cache && onTTS && onTTS({
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

            // test...
            // const tts_cache_key = `${device_id}_${text}`;
            // const cache_TTS = G_get_cahce_TTS(tts_cache_key);
            // if (cache_TTS && !is_cache) {
            //     console.log('使用缓存 TTS ')
            //     return ()=> new Promise((resolve)=>{
            //         cb({
            //             audio: cache_TTS,
            //             is_over: true,
            //             tts_task_id, device_id, session_id, text_is_over, need_record, frameOnTTScb
            //         })
            //         resolve(true);
            //     });
            // }

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
            const connectServerCb = async (connected) => {
                if (connected) {
                    if (!G_devices.get(device_id)) return;
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        tts_server_connected: true,
                        tts_server_connect_ing: false,
                    })

                    !is_create_cache && ws_client && ws_client.send(JSON.stringify({
                        type: "session_status",
                        status: "tts_chunk_start",
                    })); 
                    // 启动音频发送任务 
                    audio_sender.startSend(tts_task_id === "connected_reply" ? "0001" : session_id, () => {
                        resolve(true);
                    });
                } else {
                    if (!G_devices.get(device_id)) {
                        return  resolve(true);
                    };
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        tts_server_connected: false,
                        tts_server_connect_ing: false,
                    })
                    !is_create_cache && ws_client && ws_client.send(Buffer.from(G_session_ids.tts_all_end, 'utf-8'));
                }
            }

            /**
             * tts 服务发生错误时调用
            */
            const ttsServerErrorCb = (err, code) => {
                error_catch("TTS", code || "302", err);
                tts_list.delete(tts_task_id)
                log.error(err)

                resolve(true);
            }

            ws_client && ws_client.send(JSON.stringify({ type: "play_audio", tts_task_id }));
            TTS_FN({
                text,
                device_id,
                devLog,
                tts_config,
                tts_params_set,
                log,
                iat_server, llm_server, tts_server,
                cb: (arg) => cb({
                    ...arg, tts_task_id, device_id, session_id, text_is_over, need_record, frameOnTTScb, is_cache, is_create_cache, audio_sender
                }),
                logWSServer,
                ttsServerErrorCb,
                connectServerBeforeCb,
                connectServerCb,
            })
        } catch (err) {
            console.log(err);
            log.error(`TTS 错误： ${err}`)
        }

    })
};
module.exports = TTSFN;
