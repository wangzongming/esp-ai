 /** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
// 发送到板子音频的每个分片的最大字节
const maxChunkSize = 256;

/**
 * @param {Buffer} is_over  是否完毕
 * @param {Buffer} audio    音频流
 * @param {WebSocket} curTTSKey    WebSocket 连接key
 * @param {WebSocket} curTTSWs    WebSocket 连接
 * @param {Function} TTS_resolve  TTS 函数的 resolve 参数
*/
async function cb({ device_id, is_over, audio, curTTSWs, curTTSKey, TTS_resolve, reRecord }) {
    const { devLog, onTTScb } = G_config;
    const { ws: ws_client, tts_list, add_audio_out_over_queue } = G_devices.get(device_id);
    onTTScb && onTTScb({ device_id, is_over, audio });
    /**
     * 1. TTS 转换完毕，并且发往客户端
     * 2. 客户端告知服务已经完成音频流播放
     * 3. 本任务完成
    */
    if (is_over) {
        devLog && console.log('-> TTS 转换完毕');
        curTTSWs.close && curTTSWs.close()
        tts_list.delete(curTTSKey)

        // 一旦TTS任务顺序混乱，那这里必定出问题。
        add_audio_out_over_queue(() => {
            const { ws: ws_client, start_iat } = G_devices.get(device_id);
            devLog && console.log('-> TTS 客户端播放完毕');
            if (reRecord) {
                devLog && console.log('\n\n=== 重新开始识别音频 ===')
                ws_client && ws_client.send("start_voice");
                start_iat && start_iat();
            }
            TTS_resolve(true);
        })
    }

    /**
     * 循环发送每个分片
     * 最后一片时需要告知客户端 ing...
     */
    for (let i = 0; i < audio.length; i += maxChunkSize) {
        const end = Math.min(i + maxChunkSize, audio.length);
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
    const { tts_server } = G_config;
    const TTS_FN = require(`./${tts_server}`);
    return TTS_FN(device_id, { ...opts, cb })
};
