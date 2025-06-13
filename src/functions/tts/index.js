/*
 * MIT License
 *
 * Copyright (c) 2025-至今 小明IO
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
        const { ws: ws_client, tts_list, session_id: now_session_id } = G_devices.get(device_id);
        if (!ws_client) return; 
        if (
            !is_create_cache &&
            session_id &&
            now_session_id &&
            session_id !== now_session_id &&
            !([G_session_ids["tts_fn"]].includes(session_id))
        ) return; 

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
            devLog && log.tts_info('-> TTS 片段转换完毕');
            ws.close && ws.close();
            tts_list.delete(tts_task_id);
            function sendEndBuffer() {
                devLog && log.tts_info(`-> 服务端发送 LLM 结束的标志流: ${G_session_ids["tts_all_end"]}`);
                audio_sender.sendAudio(Buffer.from(G_session_ids["tts_all_end"], 'utf-8'));
            }
            function sendEndAlignBuffer() {
                audio_sender.sendAudio(Buffer.from(G_session_ids["tts_all_end_align"], 'utf-8'));
            }


            if (text_is_over) {
                /**
                 * 意图推理中时候并不知道是否还需要重新采集音频
                 * 所以需要等待推理完毕后才进行最后一帧音频流的发送
                */
                G_Instance.awaitIntention(device_id, () => {
                    if (!G_devices.get(device_id)) return;
                    const { stop_next_session } = G_devices.get(device_id);
                    if (!stop_next_session) {
                        if (need_record) {
                            sendEndAlignBuffer();
                        } else {
                            sendEndBuffer();
                        }
                    } else {
                        sendEndBuffer();
                    }
                })
            } else {
                // 文本没有结束，发送 chunk 标识
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
    try {
        return new Promise((resolve) => {
            try {
                if (!G_devices.get(device_id)) return;
                const { devLog, plugins = [], tts_params_set, onTTS } = G_config;
                const {
                    ws: ws_client, error_catch, tts_list,
                    user_config: { iat_server, llm_server, tts_server, tts_config }
                } = G_devices.get(device_id);
                const { text, is_pre_connect, session_id, text_is_over = true, need_record = false, frameOnTTScb, is_cache, is_create_cache, tts_task_id = createUUID() } = opts;
                const plugin = plugins.find(item => item.name === tts_server && item.type === "TTS")?.main;

                const TTS_FN = plugin || require(`./${tts_server}`);
                if (!is_pre_connect) {
                    if (!text || !(`${text}`.replace(/\s/g, ''))) {
                        return true;
                    }
                }
                if (is_create_cache) {
                    devLog && log.tts_info('-> 开始缓存TTS: ', text);
                } else {
                    devLog && log.tts_info('-> 开始请求TTS: ', text);
                }

                const audio_sender = new Audio_sender(ws_client, device_id);

                !is_create_cache && onTTS && onTTS({
                    device_id, tts_task_id,
                    ws: ws_client,
                    text,
                    text_is_over,
                    instance: G_Instance,
                    sendToClient: (_text) => ws_client && ws_client.send(JSON.stringify({
                        type: "instruct",
                        command_id: "on_tts",
                        data: _text || text
                    }))
                });

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
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        tts_server_connect_ing: true,
                        // 记录下来，如果设备打断时，需要框架调用，否则本任务将永远无法结束
                        resolve_tts_task: resolve
                    })
                }

                /**
                * 连接 tts 服务后的回调
                */
                const connectServerCb = async (connected) => {
                    if (connected) {
                        if (!G_devices.get(device_id)) return;
                        devLog && log.tts_info("-> TTS 服务连接成功！")
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            audio_sender: audio_sender,
                            tts_server_connected: true,
                            tts_server_connect_ing: false,
                        })

                        !is_create_cache && ws_client && ws_client.send(JSON.stringify({
                            type: "session_status",
                            status: "tts_chunk_start",
                        }));
                        // 启动音频发送任务 
                        audio_sender.startSend(tts_task_id === "connected_reply" ? "0001" : session_id, () => {
                            G_devices.set(device_id, {
                                ...G_devices.get(device_id),
                                resolve_tts_task: null,
                                audio_sender: null,
                            })
                            resolve(true);
                        });
                    } else {
                        if (!G_devices.get(device_id)) {
                            G_devices.set(device_id, {
                                ...G_devices.get(device_id),
                                resolve_tts_task: null
                            })
                            return resolve(true);
                        };
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            tts_server_connected: false,
                            tts_server_connect_ing: false,
                        })
                    }
                }

                /**
                 * tts 服务发生错误时调用
                */
                const ttsServerErrorCb = (err, code) => {
                    error_catch("TTS", code || "302", err);
                    tts_list.delete(tts_task_id)
                    log.error(err)

                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        resolve_tts_task: null
                    })
                    resolve(true);
                }

                ws_client && ws_client.send(JSON.stringify({ type: "play_audio", tts_task_id }));
                TTS_FN({
                    text,
                    device_id,
                    session_id,
                    is_pre_connect,
                    devLog,
                    tts_config,
                    tts_params_set,
                    log,
                    iat_server, llm_server, tts_server, text_is_over,
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
    } catch (err) {
        console.error(`tts/index.js 错误：`, err)
    }

};
module.exports = TTSFN;
