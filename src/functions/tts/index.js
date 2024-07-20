/** 
* @author xiaomingio 
* @github https://github.com/wangzongming/esp-ai  
*/
const play_temp = require(`../../audio_temp/play_temp`);
const log = require("../../utils/log");
const createUUID = require("../../utils/createUUID");

/**
 * @param {Buffer}    is_over       是否完毕
 * @param {Buffer}    audio         音频流
 * @param {WebSocket} tts_task_id   WebSocket 连接key
 * @param {WebSocket} ws            WebSocket 连接
 * @param {Function}  resolve       TTS 函数的 resolve 参数
*/
async function cb({ device_id, is_over, audio, ws, tts_task_id, resolve, reRecord }) {
    const { devLog, onTTScb } = G_config;
    const { ws: ws_client, tts_list, add_audio_out_over_queue } = G_devices.get(device_id);
    onTTScb && onTTScb({ device_id, is_over, audio });
    if (!resolve) {
        log.error('TTS 插件中，调用 cb 时 resolve 参数不能为空');
    }
    /**
     * 1. TTS 转换完毕，并且发往客户端
     * 2. 客户端告知服务已经完成音频流播放
     * 3. 本任务完成
    */
    if (is_over) {
        devLog && log.tts_info('-> TTS 转换完毕');
        ws.close && ws.close()
        tts_list.delete(tts_task_id)

        add_audio_out_over_queue(tts_task_id, async () => {
            const { ws: ws_client, start_iat } = G_devices.get(device_id);
            if (reRecord) { 
                add_audio_out_over_queue("warning_tone", () => { 
                    start_iat && start_iat();
                    resolve(true);
                })
                await play_temp("du.pcm", ws_client); 
            }else{ 
                resolve(true);
            }
        })
    }

    /**
     * 循环发送每个分片 
     */
    let isFirst = true;
    let c_l = isFirst ? G_max_audio_chunk_size * 2 : G_max_audio_chunk_size;
    for (let i = 0; i < audio.length; i += c_l) {
        isFirst = false;
        const end = Math.min(i + c_l, audio.length);
        // 切分缓冲区
        const chunk = audio.slice(i, end);
        ws_client.send(chunk);
    }

}

/**
 * TTS 模块
 * @param {String} device_id 设备id 
 * @param {String} text 待播报的文本 
 * @param {Boolean} pauseInputAudio 客户端是否需要暂停音频采集
 * @param {Boolean} reRecord TTS播放完毕后是再次进入iat识别环节
 * @return {Function} (pcm)=> Promise<Boolean>
*/
module.exports = (device_id, opts) => {
    const { devLog, api_key, tts_server, plugins = [], tts_params_set } = G_config;
    const { ws: ws_client, tts_list, add_audio_out_over_queue } = G_devices.get(device_id);
    const { text, pauseInputAudio, reRecord, onAudioOutOver } = opts;
    const plugin = plugins.find(item => item.name == tts_server && item.type === "TTS")?.main;

    const TTS_FN = plugin || require(`./${tts_server}`);
    if (!api_key[tts_server]) {
        log.error(`请先配置好 TTS 服务 ${tts_server} 的 api_key`);
        return false;
    }
    if (!text) {
        return true;
    }
    devLog && log.tts_info("");
    devLog && log.tts_info('=== 开始请求TTS: ', text, " ===");

    // 开始播放直接先让 esp32 暂停采集音频，不然处理不过来
    if (pauseInputAudio) {
        ws_client && ws_client.send("pause_voice");
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            client_out_audio_ing: true,
        })
    }

    // 任务ID
    const tts_task_id = createUUID();
    onAudioOutOver && add_audio_out_over_queue(tts_task_id, onAudioOutOver)

    /**
     * 记录 tts 服务对象
    */
    const logWSServer = (wsServer) => {
        tts_list.set(tts_task_id, wsServer)
    }

    /**
     * tts 服务发生错误时调用
    */
    const ttsServerErrorCb = (err) => {
        tts_list.delete(tts_task_id)
        log.error(err)
    }


    ws_client && ws_client.send(JSON.stringify({ type: "play_audio", tts_task_id }));
    return TTS_FN({
        text,
        device_id,
        devLog,
        api_key: api_key[tts_server],
        tts_params_set,
        log,
        cb: (arg) => cb({ ...arg, tts_task_id, device_id, reRecord }),
        logWSServer,
        ttsServerErrorCb
    })
};
