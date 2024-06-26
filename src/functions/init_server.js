/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const createUUID = require('./createUUID') 
const IAT_FN = require(`./iat`);
const TTS_FN = require(`./tts`);
const LLM_FN = require(`./llm`);

function init_server() {
    const { port, devLog, onDeviceConnect, f_reply } = G_config;
    const wss = new WebSocket.Server({ port });
    wss.on('connection', async function connection(ws) {
        const device_id = createUUID();
        onDeviceConnect && onDeviceConnect({ ws, device_id });

        // 音频流播放完毕的任务队列
        let audio_queue = [];
        function add_audio_out_over_queue(fn) {
            audio_queue.push(fn);
        }

        G_devices.set(device_id, { ws, first_session: true, tts_list: new Map(), await_out_tts: [], add_audio_out_over_queue })
        devLog && console.log(`\n\n硬件设备连接成功：${device_id}`);
        let started = false;

        ws.on('message', async function (data) {
            // 避免浪费性能, 否则播放会卡顿
            const { send_pcm, iat_server_connected, tts_list = [], iat_ws, llm_ws, first_session, iat_end_frame_timer, client_out_audio_ing } = G_devices.get(device_id);

            switch (data) {
                case "start":
                    if (iat_server_connected || client_out_audio_ing) {
                        // devLog && console.log("--- IAT 识别途中收到重新输入音频");
                        return;
                    };
                    // 清空 tts 任务 
                    for (const [key, ttsWS] of tts_list) {
                        ttsWS.close();
                        tts_list.delete(key)
                    }
                    iat_ws && iat_ws.close()
                    llm_ws && llm_ws.close()
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
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
                    })
                    started = true;
                    const start_iat = () => IAT_FN(device_id); 
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        start_iat: start_iat,
                    })

                    first_session && await TTS_FN(device_id, {
                        text: f_reply || "小明在的",
                        reRecord: true,
                        pauseInputAudio: true
                    });
                    !first_session && start_iat();


                    // ============= LLM 测试 =============
                    // LLM_FN(device_id, { text: "你好。" }) 
                    // LLM_FN(device_id, { text: "你好，帮我写一首现代诗，描写春色，模仿徐志摩的手笔。" }) 
                    break;
                case "client_out_audio_ing":
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        client_out_audio_ing: true,
                    })
                    break;
                case "client_out_audio_over":
                    devLog && console.log("-> 客户端音频流播放完毕");
                    for (const fn of audio_queue) {
                        await fn();
                    }
                    audio_queue = [];

                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        client_out_audio_ing: false, 
                    })
                    break;
                default:
                    // 采集的音频数据
                    if (started && data && data.length && send_pcm && iat_server_connected) {
                        // 发送数据 
                        send_pcm(data);
                        // 准备发送最后一帧
                        clearTimeout(iat_end_frame_timer);
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            iat_end_frame_timer: setTimeout(() => {
                                const { send_pcm, iat_server_connected, iat_server } = G_devices.get(device_id);
                                switch (iat_server) {
                                    case "xun_fei":
                                        if (iat_server_connected && send_pcm) {
                                            devLog && console.log("主动发送IAT最后一帧");
                                            // 最终帧发送结束 
                                            started = false;
                                            G_devices.set(device_id, {
                                                ...G_devices.get(device_id),
                                                iat_status: XF_IAT_FRAME.STATUS_LAST_FRAME,
                                            })
                                            send_pcm("");
                                        }
                                        break;
                                    default:
                                        break;
                                }
                            }, 1800) // 需要比静默时间少,
                        })
                    }
                    break;
            }
        }); 
        
        // ============= TTS 测试 =============
        // await TTS_FN(device_id, {
        //     text: "第一句，小明在的。请吩咐！很愿意为你效劳。",
        //     reRecord: false,
        //     pauseInputAudio: true
        // }); 
 

        ws.on('close', () => {
            devLog && console.log(`硬件设备断开连接: ${device_id}`);
            G_devices.delete(device_id)
        });
    });
    console.log(`WebSocket server is running on ws://localhost:${port}`);
    return wss;
}
module.exports = init_server;
