const log = require("../../utils/log");
const play_audio = require("../../audio_temp/play_audio");

/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
async function cb({ device_id, text }) {
    const { devLog, intention = [], onIATcb, sleep_reply } = G_config;
    const TTS_FN = require(`../tts`);
    const LLM_FN = require(`../llm`);
    onIATcb && onIATcb({ device_id, text });
    const { first_session, ws: ws_client } = G_devices.get(device_id);


    if (text.length) {
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            first_session: false,
        })

        let task_info = null;
        intention_for: for (const item of intention) {
            const { key = [] } = item;
            if (typeof key === "function") {
                const res = await key(text);
                if (res) {
                    task_info = item;
                    task_info["__name__"] = res;
                    break intention_for;
                }
            } else {
                if (key.includes(text)) {
                    task_info = item;
                    break intention_for;
                }
            }
        }
        if (task_info) {
            const { instruct, message, data, __name__, music_server } = task_info;
            switch (instruct) {
                case "__sleep__":
                    await TTS_FN(device_id, {
                        text: message || sleep_reply || "我先退下了，有需要再叫我。",
                        reRecord: false,
                        pauseInputAudio: true
                    });
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        first_session: true,
                    })
                    ws_client && ws_client.send("session_end");
                    devLog && console.log('\n\n === 会话结束 ===\n\n')
                    break;
                case "__play_music__":
                    await TTS_FN(device_id, {
                        text: message || "好的",
                        reRecord: false,
                        pauseInputAudio: true,
                        onAudioOutOver: async () => {
                            const url = await music_server(__name__);
                            play_audio(url, ws_client, "play_music")
                        }
                    });
                    break;
                default:
                    devLog && log.iat_info(`执行指令：${instruct}, data: ${data}, name: ${__name__}`);
                    instruct && ws_client && ws_client.send(JSON.stringify({
                        type: "instruct",
                        command_id: instruct,
                        data: data,
                        name: __name__
                    }));
                    message && await TTS_FN(device_id, {
                        text: message,
                        reRecord: true,
                        pauseInputAudio: true
                    });
                    break;
            }
        } else {
            // 其他情况交给 LLM
            LLM_FN(device_id, { text })
        }
    } else {
        !first_session && TTS_FN(device_id, {
            text: sleep_reply || "我先退下了，有需要再叫我。",
            reRecord: false,
            pauseInputAudio: true
        });
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            first_session: true,
        })
        ws_client && ws_client.send("session_end");
    }
}

module.exports = async (device_id) => {
    const TTS_FN = require(`../tts`);
    const { devLog, api_key, iat_server, plugins = [], sleep_reply } = G_config;
    devLog && log.info('');
    devLog && log.iat_info('=== 开始请求语音识别 ===');
    if (!api_key[iat_server]) {
        log.error(`请先配置好 ${iat_server} 服务的 api_key`);
        return;
    }

    const plugin = plugins.find(item => item.name == iat_server && item.type === "IAT")?.main;
    const IAT_FN = plugin || require(`./${iat_server}`);

    /**
    * 连接 iat 服务后的回到
    */
    const connectServerCb = (connected) => {
        if (connected) {
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_server_connected: true,
            })
        } else {
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_server_connected: false,
                iat_ws: null,
            })
        }
    }

    /**
    * 记录 tts 服务对象
    */
    const logWSServer = (wsServer) => {
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
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            send_pcm: sendFn
        })
    }

    /**
     * 服务发生错误时调用
    */
    const iatServerErrorCb = (err) => {
        log.error("iat error: " + err)
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            iat_ws: null,
            iat_server_connected: false,
        })
        TTS_FN(device_id, {
            text: "语音识别发生了错误。",
            reRecord: true,
            pauseInputAudio: true
        });
    }

    /**
     * 当 IAT 服务连接成功了，但是长时间不响应时
    */
    const serverTimeOutCb = () => {
        const { iat_ws, ws: ws_client, iat_end_frame_timer, iat_server_connected } = G_devices.get(device_id)
        clearTimeout(iat_end_frame_timer);
        if (!iat_server_connected) return;
        iat_ws && iat_ws.close();
        connectServerCb(false);

        devLog && log.iat_info('=== IAT服务响应超时，会话结束 ===');
        TTS_FN(device_id, {
            text: sleep_reply || "我先退下了，有需要再叫我。",
            reRecord: false,
            pauseInputAudio: true,
            onAudioOutOver: () => {
                ws_client && ws_client.send("session_end");
            }
        });
    }

    /**
     * iat 静默时间达到后触发， 一般在这里面进行最后一帧的发送，告诉服务端结束识别 
    */
    const iatEndQueueCb = (fn) => {
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            iat_end_queue: fn
        })
    }

    return IAT_FN({
        device_id,
        log,
        api_key: api_key[iat_server],
        devLog,
        vad_eos: G_vad_eos,
        cb: (arg) => cb({ ...arg, device_id }),
        iatServerErrorCb,
        logWSServer,
        connectServerCb,
        logSendAudio,
        serverTimeOutCb,
        iatEndQueueCb
    })
};
