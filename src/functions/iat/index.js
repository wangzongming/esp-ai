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
            // ws_client && ws_client.send("session_end");
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
        const { devLog, plugins = [], onIAT, onSleep } = G_config;
        const { ws: ws_client, session_id, error_catch, user_config: { iat_server, llm_server, tts_server, iat_config } } = G_devices.get(device_id)

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
                devLog && log.iat_info("-> ASR 服务连接成功: " + session_id)
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    iat_server_connected: true,
                    iat_server_connect_ing: false,
                    asr_buffer_cache: Buffer.from([]),
                })
                connected_cb && connected_cb();
                ws_client && ws_client.send(JSON.stringify({
                    type: "session_status",
                    status: "iat_start",
                }));

                const LLM_FN = require(`../llm`);
                LLM_FN(device_id, { is_pre_connect: true })
            } else {
                if (!G_devices.get(device_id)) return;
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    iat_server_connected: false,
                    iat_server_connect_ing: false,
                    iat_ws: null,
                })

                const { session_id: _session_id } = G_devices.get(device_id)
                if (session_id !== _session_id) {
                    console.log('无需发送打断标识')
                    return;
                }
                ws_client && ws_client.send(JSON.stringify({
                    type: "session_status",
                    status: "iat_end",
                }));
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

        /**
        * 记录发送音频数据给服务的函数，框架在合适的情况下会进行调用
        */
        const logSendAudio = (sendFn) => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                send_pcm: sendFn
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
            iatEndQueueCb
        })
    } catch (err) {
        console.log("IAT 错误：", err);
        log.error(`IAT 错误： ${err}`)
    }
};
