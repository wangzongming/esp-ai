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
const createUUID = require("../utils/createUUID");
const {
    audio, start, play_audio_ws_conntceed, client_out_audio_ing: client_out_audio_ing_fn,
    client_out_audio_over, cts_time, client_receive_audio_over, tts
} = require("../functions/client_messages");
const { add_audio_out_over_queue_hoc, run_audio_out_over_queue_hoc, clear_audio_out_over_queue_hoc } = require("./device_fns/audio_out_over_queue")
const error_catch_hoc = require("./device_fns/error_catch")
function init_server() {
    try {
        const { port, devLog, onDeviceConnect, auth, gen_client_config } = G_config;
        if (!gen_client_config) {
            log.error("请配置 gen_client_config 函数");
            return;
        }

        const wss = new WebSocket.Server({ port });
        wss.on('connection', async function connection(ws, req) {
            const device_id = createUUID();
            const client_params = parseUrlParams(req.url);
            const client_version = client_params.v;

            const audio_queue = new Map([]);
            G_devices.set(device_id, {
                started: false,
                ws,
                user_config: {},
                first_session: true,
                tts_list: new Map(),
                await_out_tts: [],
                client_params,
                client_version,
                add_audio_out_over_queue: add_audio_out_over_queue_hoc(audio_queue),
                run_audio_out_over_queue: run_audio_out_over_queue_hoc(audio_queue),
                clear_audio_out_over_queue: clear_audio_out_over_queue_hoc(audio_queue),
                error_catch: error_catch_hoc(ws)
            });

            onDeviceConnect && onDeviceConnect({ ws, device_id, client_version });

            ws.on('message', async function (data) {
                const comm_args = { device_id };
                try { 
                    if (typeof data === "string") {
                        const { type, tts_task_id, stc_time, session_id, sid, text } = JSON.parse(data);
                        comm_args.session_id = session_id;
                        comm_args.tts_task_id = tts_task_id;
                        comm_args.sid = sid;
                        comm_args.stc_time = stc_time;
                        comm_args.type = type;
                        comm_args.text = text;
                        switch (type) {
                            case "start":
                                start(comm_args);
                                break;
                            case "client_out_audio_ing":
                                client_out_audio_ing_fn(comm_args)
                                break;
                            case "client_out_audio_over":
                                client_out_audio_over(comm_args);
                                break;
                            case "client_receive_audio_over":
                                client_receive_audio_over(comm_args);
                                break;
                            case "play_audio_ws_conntceed":
                                play_audio_ws_conntceed(comm_args)
                                break;
                            case "tts":
                                tts(comm_args)
                                break;
                            case "cts_time":
                                cts_time(comm_args);
                                break;
                        }
                    } else {
                        audio({ ...comm_args, data })
                    }

                } catch (err) {
                    console.log(err);
                    log.error(`消息处理错误：${err}`)
                }

            });

            if (auth) {
                const { success: auth_success, message: auth_message } = await auth(client_params, "connect");
                if (!auth_success) {
                    ws.send(JSON.stringify({ type: "auth_fail", message: `${auth_message || "-"}` }));
                    ws.close();
                    return;
                };
            }

            // const IAT_FN = require(`./iat`);
            // const TTS_FN = require(`./tts`);
            // const LLM_FN = require(`./llm`);
            // ============= 提示音测试 =============
            // play_temp("du.pcm", ws, 0.8, 24);   
            // play_audio("http://m10.music.126.net/20240723180659/13eabc0c9291dab9a836120bf3f609ea/ymusic/5353/0f0f/0358/d99739615f8e5153d77042092f07fd77.mp3", ws)


            // ============= 指令发送测试 ============= 
            // ws.send(JSON.stringify({ type: "instruct", command_id: "open_test", data: "这是数据" }));


            // ============= TTS 测试 =============  
            // await TTS_FN(device_id, {
            //     text: "小明在的",
            //     reRecord: false,
            //     pauseInputAudio: true,
            //     onAudioOutOver: () => {
            //         console.log('第一句播放完毕的回调')
            //     }
            // });

            // await new Promise(resolve => setTimeout(resolve, 1000)); 

            // await TTS_FN(device_id, {
            //     text: "第二句，萌娃音色要上线啦！",
            //     reRecord: false,
            //     pauseInputAudio: true,
            //     onAudioOutOver: () => {
            //         console.log('第二句播放完毕的回调')
            //     }
            // });
            // await new Promise(resolve => setTimeout(resolve, 1000)); 

            // await TTS_FN(device_id, {
            //     text: "第三句！第三句！第三句！",
            //     reRecord: false,
            //     pauseInputAudio: true,
            //     onAudioOutOver: () => {
            //         console.log('第三句播放完毕的回调')
            //     }
            // });


            ws.on('close', (code, reason) => {
                devLog && log.info(``);
                devLog && log.t_red_info(`硬件设备断开连接: ${device_id}， code: ${code}， reason: ${reason}`);
                devLog && log.info(``);
                G_devices.delete(device_id)
            });
            ws.on('error', function (error) {
                log.error(`WebSocket Client error: ${error.toString()}`);
            });
        });
        wss.on('error', function (error) {
            log.error(`WebSocket server error: ${error.toString()}`);
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
