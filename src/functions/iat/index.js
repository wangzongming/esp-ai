const log = require("../../utils/log");

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

const { clear_sid } = require("../device_fns");
async function cb({ device_id, text }) {
    try {
        const { onIATcb, onSleep } = G_config;
        const LLM_FN = require(`../llm`);
        if (!G_devices.get(device_id)) return;
        const { ws: ws_client, client_params, user_config: { sleep_reply } } = G_devices.get(device_id);

        ws_client && ws_client.send(JSON.stringify({
            type: "log",
            data: `ASR 识别结果：${text}`
        }))

        onIATcb && onIATcb({
            device_id, text, ws: ws_client,
            instance: G_Instance,
            sendToClient: (_text) => ws_client && ws_client.send(JSON.stringify({
                type: "instruct",
                command_id: "on_iat_cb",
                data: _text || text
            }))
        });


        if (text.length) {
            G_Instance.matchIntention(device_id, text);
            LLM_FN(device_id, { text })
        } else {
            onSleep && onSleep({ instance: G_Instance, device_id, client_params });
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                first_session: true,
            })

            // 超时结束
            ws_client && ws_client.send(JSON.stringify({ type: "session_status", status: "iat_end" }));
            ws_client && ws_client.send("session_end", () => {
                G_Instance.tts(device_id, sleep_reply);
            });
        }
    } catch (err) {
        console.log("IAT 回调错误：", err);
        log.error(`[${device_id}] IAT 回调错误： ${err}`)
    }
}

module.exports = async (device_id, connected_cb) => {
    try {
        const TTS_FN = require(`../tts`);
        const { devLog, plugins = [], onIAT, onSleep, vad_first, vad_course, api_key: g_api_key, ai_server } = G_config;
        const { ws: ws_client, api_key: user_api_key, session_id, error_catch, user_config: { iat_server, llm_server, tts_server, iat_config }, llm_historys = [] } = G_devices.get(device_id)

        const api_key = user_api_key || g_api_key;
        let prev_asr_text = ""; // 上一次识别出来的文字  
        let vad_ended = false;     // vad 结束
        let asr_timeouter = null;     // vad 结束 

        devLog && log.info('');
        devLog && log.iat_info('-> 开始请求语音识别');

        const plugin = plugins.find(item => item.name === iat_server && item.type === "IAT")?.main;
        const IAT_FN = plugin || require(`./${iat_server}`);
        onIAT && onIAT({ device_id, ws: ws_client });


        /**
         * 开始连接 iat 服务的回调
         */
        const connectServerBeforeCb = () => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_server_connect_ing: true,
            })
        }

        /**
        * 连接 iat 服务后的回调
        */
        const connectServerCb = (connected) => {
            if (!G_devices.get(device_id)) return;
            if (connected) {
                if (!G_devices.get(device_id)) return;

                // asr 超时兜底
                asr_start_time = +new Date();
                clearTimeout(asr_timeouter);
                asr_timeouter = setTimeout(() => {
                    if (!G_devices.get(device_id)) return;
                    const { iat_ws } = G_devices.get(device_id)
                    if (!iat_ws) return; 
                    clear_sid(device_id);
                    iat_ws.end && iat_ws.end();
                    vad_ended = true;
                }, vad_first);

                devLog && log.iat_info("-> ASR 服务连接成功: " + session_id)
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    iat_server_connected: true,
                    iat_server_connect_ing: false,
                    asr_buffer_cache: Buffer.from([]),
                })
                connected_cb && connected_cb();

                // 告诉硬件开始发送音频数据
                ws_client && ws_client.send(JSON.stringify({
                    type: "session_status",
                    status: "iat_start",
                }));

                const LLM_FN = require(`../llm`);
                LLM_FN(device_id, { is_pre_connect: true })
            } else {
                if (!G_devices.get(device_id)) return;
                clearTimeout(asr_timeouter);
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    iat_server_connected: false,
                    iat_server_connect_ing: false,
                    iat_ws: null,
                })
                const { session_id: _session_id } = G_devices.get(device_id) 
                if (session_id !== _session_id) {
                    return;
                }
                ws_client && ws_client.send(JSON.stringify({ type: "session_status", status: "iat_end" }));
            }
        }

        /**
        * 记录 tts 服务对象
        */
        const logWSServer = (wsServer) => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_ws: wsServer,
                iat_server_connected: false,
            })
        }

        const logSendAudio = (send_pcm) => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                send_pcm
            })
        }

        /**
         * 服务发生错误时调用
        */
        const iatServerErrorCb = (err, code) => {
            if (!G_devices.get(device_id)) return;
            log.error("IAT error: " + err)
            error_catch("IAT", code || "102", err);
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_ws: null,
                iat_server_connected: false,
            })
            TTS_FN(device_id, {
                text: `语音识别发生了错误:${err}`,
                reRecord: true,
                pauseInputAudio: true
            });
        }

        /**
         * 当 IAT 服务连接成功了，但是长时间不响应时
        */
        const serverTimeOutCb = () => {
            const { iat_ws, client_params, iat_server_connected } = G_devices.get(device_id)
            if (!iat_server_connected) return;
            iat_ws && iat_ws.close();
            connectServerCb(false);

            onSleep && onSleep({ instance: G_Instance, device_id, client_params });
            devLog && log.iat_info('-> IAT服务响应超时，会话结束');
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                stoped: true,
            })
        }

        /**
         * iat 静默时间达到后触发， 一般在这里面进行最后一帧的发送，告诉服务端结束识别
        */
        const iatEndQueueCb = (fn) => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                stoped: true,
                iat_end_queue: fn
            })
        }

        /**
         * 语音识别的回调，只要客户端发送音频数据到了服务端，服务端转送到了 ASR 后就一定会触发本函数
        */
        let done_timer = null;
        let not_done_timer = null;
        let on_iat_text_time = null;
        const onIATText = (text) => {
            if (vad_ended) return;
            if (text !== prev_asr_text) {
                clearTimeout(asr_timeouter);

                prev_asr_text = text;
                on_iat_text_time = +new Date();
                clearTimeout(not_done_timer);
                clearTimeout(done_timer);
                done_timer = setTimeout(() => {
                    const _on_iat_text_time = on_iat_text_time;
                    done_talking({ ai_server, text, api_key, prev_text: llm_historys?.[llm_historys.length - 1]?.content }).then((score) => {
                        if (_on_iat_text_time !== on_iat_text_time) { 
                            // log.t_red_info("放弃本次语义判断。")
                            return;
                        }
                        const done = score >= 60;
                        // log.t_red_info(text, score, done);

                        const over_do = () => {
                            if (!G_devices.get(device_id)) return;
                            const { iat_ws } = G_devices.get(device_id)
                            if (!iat_ws) return;
                            iat_ws.end && iat_ws.end();
                            vad_ended = true;
                        }
                        if (done) {
                            // log.t_red_info("语义结束")
                            over_do();
                        } else {
                            clearTimeout(not_done_timer);
                            not_done_timer = setTimeout(() => {
                                log.t_red_info("超时结束 VAD ")
                                over_do();
                            }, 2000)
                        }
                    });
                }, vad_course);
            }

        }



        return IAT_FN({
            session_id,
            device_id,
            log,
            iat_config: iat_config,
            devLog,
            iat_server, llm_server, tts_server,
            cb: (arg) => cb({ ...arg, device_id }),
            iatServerErrorCb,
            logWSServer,
            connectServerBeforeCb,
            connectServerCb,
            logSendAudio,
            serverTimeOutCb,
            iatEndQueueCb,
            onIATText
        })
    } catch (err) {
        console.log("IAT 错误：", err);
        log.error(`IAT 错误： ${err}`)
    }
};

/**
* 判断语义是否完整
*/
const axios = require('axios');
async function done_talking({ ai_server, text, api_key, prev_text }) {
    try {
        if (!text) return false;
        const response = await axios.post(`${ai_server}/ai_api/done_talking`, { text, api_key, prev_text }, { headers: { 'Content-Type': 'application/json' } });
        const { success, message: res_msg, data } = response.data;
        if (success) {
            return data;
        } else {
            log.error('-> 语义推理失败：' + res_msg);
            return false;
        }
    } catch (err) {
        log.error('-> 语义推理失败，请求失败：');
        console.log(err);
        return false;
    }
}
