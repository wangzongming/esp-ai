/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const play_temp = require('../audio_temp/play_temp')
const play_audio = require('../audio_temp/play_audio')
const IAT_FN = require(`./iat`);
const TTS_FN = require(`./tts`);
const LLM_FN = require(`./llm`);
const log = require("../utils/log");
const getIPV4 = require("../utils/getIPV4");
const parseUrlParams = require("../utils/parseUrlParams");
const createUUID = require("../utils/createUUID");
const { tts_info, iat_info, llm_info, info } = require("../utils/log");

function init_server() {
    const { port, devLog, onDeviceConnect, f_reply, onIATEndcb, auth } = G_config; 
    const wss = new WebSocket.Server({ port });
    wss.on('connection', async function connection(ws, req) {
        const device_id = createUUID();
        const client_params = parseUrlParams(req.url);
        const client_version = client_params.v;
        const { success: auth_success, message: auth_message } = await auth(client_params, "connect");
        if (!auth_success) {
            ws.send(JSON.stringify({ type: "auth_fail", message: `${auth_message || "-"}` }));
            ws.close();
            return;
        };

        // TTS 音频流播放完毕的任务队列 
        const audio_queue = new Map([]);

        function add_audio_out_over_queue(key, fn) {
            if (!key) {
                log.error("add_audio_out_over_queue key is null");
                return;
            }
            const fns = audio_queue.get(key) || [];
            fns.push(fn)
            audio_queue.set(key, fns);
        }

        async function run_audio_out_over_queue(key) {
            const fns = audio_queue.get(key);
            if (fns) {
                for (const fn of fns) {
                    await fn();
                }
                audio_queue.delete(key);
            }
        }

        G_devices.set(device_id, {
            ws,
            first_session: true,
            tts_list: new Map(),
            await_out_tts: [],
            client_params,
            add_audio_out_over_queue,
            run_audio_out_over_queue,
        })

        devLog && log.info(`\n硬件连接成功：${device_id}`, ["bold"]);
        devLog && log.info(`客户端版本号：v${client_version}\n`, ["bold"]);

        onDeviceConnect && onDeviceConnect({ ws, device_id, client_version });

        let started = false;
        ws.on('message', async function (data) {
            // 避免浪费性能, 否则播放会卡顿
            const { send_pcm, iat_server_connected, tts_list = [], iat_ws, llm_ws, first_session, iat_end_frame_timer, client_out_audio_ing, alert_ing, iat_end_queue } = G_devices.get(device_id);
            if (typeof data === "string") {
                const { type, tts_task_id } = JSON.parse(data);
                // console.log(JSON.parse(data));
                switch (type) {
                    case "start":
                        const { success: auth_success, message: auth_message } = await auth(client_params, "start_session");
                        if (!auth_success) {
                            ws.send(JSON.stringify({ type: "auth_fail", message: `${auth_message || "-"}` }));
                            ws.close();
                            return;
                        };
                        if (iat_server_connected || client_out_audio_ing) {
                            // devLog && console.log("--- IAT 识别途中收到重新输入音频");
                            return;
                        };
                        // 清空 tts 任务 
                        for (const [key, ttsWS] of tts_list) {
                            ttsWS && ttsWS.close && ttsWS.close();
                            tts_list.delete(key)
                        }
                        iat_ws && iat_ws.close && iat_ws.close()
                        llm_ws && llm_ws.close && llm_ws.close()

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
                        const start_iat = () => {
                            ws && ws.send("start_voice");
                            started = true;
                            return IAT_FN(device_id);
                        };
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            start_iat: start_iat,
                        })

                        if (first_session) {
                            TTS_FN(device_id, {
                                text: f_reply || "小明在的",
                                reRecord: true,
                                pauseInputAudio: true
                            });
                        } else {
                            add_audio_out_over_queue("warning_tone", () => {
                                start_iat();
                            })
                            await play_temp("du.pcm", ws);
                        }

                        // ============= LLM 测试 =============
                        // LLM_FN(device_id, { text: "你好。" }) 
                        // LLM_FN(device_id, { text: "你好，帮我写一首现代诗，描写春色，模仿徐志摩的手笔。" }) 
                        break;
                    case "client_out_audio_ing":
                        // if (alert_ing) return;
                        devLog && tts_info("-> 客户端音频流播放中");
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            client_out_audio_ing: true,
                        })
                        break;
                    case "client_out_audio_over":
                        devLog && tts_info("-> 客户端音频流播放完毕：", tts_task_id);
                        run_audio_out_over_queue(tts_task_id);
                        if (tts_task_id === "play_music") {
                            ws && ws.send("session_end");
                        }
                        G_devices.set(device_id, {
                            ...G_devices.get(device_id),
                            client_out_audio_ing: false,
                        })
                        break;
                    case "play_audio_ws_conntceed":
                        // 播放ws连接成功语音
                        await TTS_FN(device_id, {
                            text: `后台服务连接成功，呼喊小明同学就可以唤醒我。`,
                            reRecord: false,
                            pauseInputAudio: true,
                            onAudioOutOver: () => {
                                ws && ws.send("session_end");
                            }
                        })
                        break;
                }
            } else {
                // 采集的音频数据
                if (started && data && data.length && send_pcm && iat_server_connected) {
                    // 发送数据 
                    send_pcm(data);
                    // 准备发送最后一帧
                    clearTimeout(iat_end_frame_timer);
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        iat_end_frame_timer: setTimeout(async () => {
                            const { iat_server_connected } = G_devices.get(device_id);
                            started = false;
                            if (!iat_server_connected) {
                                return;
                            }
                            devLog && iat_info("IAT 超时未收到音频数据，执行主动结束回调。");
                            iat_end_queue && await iat_end_queue();
                            onIATEndcb && await onIATEndcb(device_id);
                        }, G_vad_eos - 300) // 需要比静默时间少,
                    })
                }
            }

        });

        // ============= 提示音测试 =============
        // play_temp("du.pcm", ws);  
        // play_audio("http://m10.music.126.net/20240723180659/13eabc0c9291dab9a836120bf3f609ea/ymusic/5353/0f0f/0358/d99739615f8e5153d77042092f07fd77.mp3", ws)


        // ============= 指令发送测试 ============= 
        // ws.send(JSON.stringify({ type: "instruct", command_id: "open_test", data: "这是数据" }));


        // ============= TTS 测试 =============  
        // await TTS_FN(device_id, {
        //     text: "第一句，小明在的！",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第一句播放完毕的回调')
        //     }
        // }); 

        // await TTS_FN(device_id, {
        //     text: "第二句，萌娃音色要上线啦！",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第二句播放完毕的回调')
        //     }
        // });


        ws.on('close', () => {
            devLog && console.log(`硬件设备断开连接: ${device_id}`);
            G_devices.delete(device_id)
        });
    });
    log.info(`---------------------------------------------------`);
    log.info(`- Github  https://github.com/wangzongming/esp-ai`, ["bold"]);
    log.info(`- Website https://xiaomingio.top/esp-ai`, ["bold"]);
    log.info(`- Server  ${getIPV4()}:${port}(copy to example.ino)`, ["bold"]);
    log.info(`---------------------------------------------------`);
    return wss;
}
module.exports = init_server;
