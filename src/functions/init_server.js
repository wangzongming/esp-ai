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
const WebSocket = require('ws')
const log = require("../utils/log");
const getIPV4 = require("../utils/getIPV4");
const parseUrlParams = require("../utils/parseUrlParams");
const isOutTimeErr = require("../utils/isOutTimeErr");
const TTS_buffer_chunk_queue = require("../utils/tts_buffer_chunk_queue");
const {
    audio, start, play_audio_ws_conntceed, client_out_audio_ing: client_out_audio_ing_fn,
    client_out_audio_over, cts_time, set_wifi_config_res, digitalRead, analogRead, iat_end
} = require("../functions/client_messages");
const error_catch_hoc = require("./device_fns/error_catch")

// 音频测试
// const fs = require('fs');
// const path = require('path');
// var index = 0;
// var writeStream;

function init_server() {
    try {
        const { port, devLog, onDeviceConnect, onDeviceDisConnect, auth, gen_client_config } = G_config;
        if (!gen_client_config) {
            log.error("请配置 gen_client_config 函数");
            return;
        }

        const wss = new WebSocket.Server({ port });
        wss.on('connection', async function connection(ws, req) {
            const client_params = parseUrlParams(req.url);
            const client_version = client_params.v;
            const device_id = client_params.device_id;
            if (!device_id) {
                log.error("设备异常，未读取到 device_id");
                setTimeout(() => {
                    ws.send(JSON.stringify({ type: "error", message: `设备异常，未读取到 device_id`, code: "004" }));
                    ws.close();
                }, 5000)
                return;
            }
            log.t_info(`[${device_id}] 硬件连接`)

            // 断电重连
            if (G_devices.get(device_id)) {
                const { ws: _ws } = G_devices.get(device_id);
                await G_Instance.stop(device_id, "打断会话时");
                ws.terminate();
                G_devices.delete(device_id);
            }

            G_devices.set(device_id, {
                started: false,
                // 会话是否已经停止, 作为 started 的辅助
                stoped: true,
                ws,
                user_config: {},
                first_session: true,
                llm_historys: [],
                tts_list: new Map(),
                await_out_tts: [],
                client_params,
                client_version,
                error_catch: error_catch_hoc(ws),
                tts_buffer_chunk_queue: new TTS_buffer_chunk_queue(device_id),
                // 已输出流量 kb
                useed_flow: 0,
                read_pin_cbs: new Map(),
            });

            ws.isAlive = true;
            ws.device_id = device_id;
            ws.client_params = client_params;

            onDeviceConnect && onDeviceConnect({
                ws, device_id, client_version, client_params,
                instance: G_Instance
            });

            ws.on('message', async function (data) {
                const comm_args = { device_id };
                try {
                    if (typeof data === "string") {
                        const { type, tts_task_id, stc_time, session_id, sid, text, success, value, pin } = JSON.parse(data);
                        comm_args.session_id = session_id;
                        comm_args.tts_task_id = tts_task_id;
                        comm_args.sid = sid;
                        comm_args.stc_time = stc_time;
                        comm_args.type = type;
                        comm_args.text = text;
                        comm_args.success = success;
                        comm_args.value = value;
                        comm_args.pin = pin;
                        switch (type) {
                            case "start":
                                start(comm_args);
                                // test...
                                // writeStream = fs.createWriteStream(path.join(__dirname, `./${index}_output.mp3`));
                                // index++;
                                break;
                            case "iat_end":
                                iat_end(comm_args);
                                break;
                            case "client_out_audio_ing":
                                client_out_audio_ing_fn(comm_args)
                                break;
                            case "client_out_audio_over":
                                client_out_audio_over(comm_args);
                                break;
                            case "play_audio_ws_conntceed":
                                play_audio_ws_conntceed(comm_args)
                                break;
                            case "tts":
                                G_Instance.tts(device_id, text)
                                break;
                            case "cts_time":
                                cts_time(comm_args);
                                break;
                            case "set_wifi_config_res":
                                set_wifi_config_res(comm_args);
                                break;
                            case "digitalRead":
                                digitalRead(comm_args);
                                break;
                            case "analogRead":
                                analogRead(comm_args);
                                break;
                        }
                    } else {
                        ws.isAlive = true;
                        audio({ ...comm_args, data })

                        // test...
                        // writeStream.write(data);
                    }

                } catch (err) {
                    console.log(err);
                    log.error(`消息处理错误：${err}`)
                }

            });

            ws.on("pong", function () {
                // console.log("收到 pong")
                this.isAlive = true;
            });


            if (auth) {
                const { success: auth_success, message: auth_message, code: auth_code } = await auth({
                    ws,
                    client_params: client_params,
                    type: "connect",
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
                        G_devices.delete(device_id);
                    }, 5000)
                    return;
                };
            }

            ws.on('close', (code, reason) => { 
                devLog && log.info(``);
                devLog && log.t_red_info(`硬件设备断开连接: ${device_id}， code: ${code}， reason: ${reason}`);
                devLog && log.info(``);
                onDeviceDisConnect && onDeviceDisConnect({ device_id, client_params, instance: G_Instance });
 
                G_Instance.stop(device_id, "设备断开服务时"); 
                G_devices.delete(device_id);
            });
            ws.on('error', function (error) {
                log.error(`WebSocket Client error: ${error.toString()}`);
            });
        });
        wss.on('error', function (error) {
            log.error(`WebSocket server error: ${error.toString()}`);
        });

        /**
         * 设备拔电的情况无法正确发送 close 事件，所以需要手动实现
         * 活动检测一定不能太快，性能是一方面
         * 主要还是在发送长音频时无法发送 ping 控制帧，如果时间过短会导致断连
        */
        const interval = setInterval(function ping() {
            wss.clients.forEach(function each(ws) {
                const bufferedAmount = ws.bufferedAmount.valueOf()
                if (bufferedAmount === 0 && ws.isAlive === false) {
                    onDeviceDisConnect && onDeviceDisConnect({ device_id: ws.device_id, client_params: ws.client_params, instance: G_Instance });
                    log.t_info(`[${ws.device_id}] 设备掉线了，关闭连接`);
                    return ws.terminate()
                };

                ws.isAlive = false;
                ws.ping();
            });
        }, 60 * 1000);

        setInterval(function () {
            log.info("当前客户端数量：" + wss.clients.size)
        }, 30 * 1000);

        wss.on('close', function close() {
            clearInterval(interval);
        });

        const ips = getIPV4();
        log.info(`---------------------------------------------------`);
        log.info(`- Github  https://github.com/wangzongming/esp-ai`, ["bold"]);
        log.info(`- Website https://espai.fun`, ["bold"]);
        log.info(`- Server Address: (Select the correct address to copy to example.ino)`, ["bold"]);
        ips.forEach((ip) => {
            log.info(`  -> ${ip}:${port}`);
        })
        log.info(``);
        log.info(`客户端未自动连接时，重新为客户端上电即可！`);
        log.info(`---------------------------------------------------`);
        return wss;
    } catch (err) {
        console.log(err);
        log.error(`初始化服务失败`);
    }
}
module.exports = init_server;
