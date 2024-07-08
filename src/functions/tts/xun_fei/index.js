/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const getServerURL = require("../../getServerURL");

/**
 * TTS 模块
 * @param {String} device_id 设备id 
 * @param {String} text 待播报的文本 
 * @param {Boolean} pauseInputAudio 客户端是否需要暂停音频采集
 * @param {Boolean} reRecord TTS播放完毕后是再次进入iat识别环节
 * @param {Function} (is_over, audio, TTS_resolve)=> void 音频回调
 * @return {Function} (pcm)=> Promise<Boolean>
*/
function TTS_FN(device_id, { text, reRecord = false, pauseInputAudio = true, cb }) {
    const { devLog, api_key, tts_server, tts_params_set } = G_config;

    const config = {
        appid: api_key[tts_server].appid,
    }

    return new Promise((resolve) => {
        devLog && console.log('\n=== 开始请求TTS: ', text, " ===");
        const { ws: ws_client, tts_list } = G_devices.get(device_id);
        // 开始播放直接先让 esp32 暂停采集音频，不然处理不过来
        if (pauseInputAudio) {
            ws_client && ws_client.send("pause_voice");
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                client_out_audio_ing: true,
            })
        }

        const curTTSKey = tts_list.size;
        const curTTSWs = new WebSocket(getServerURL("TTS"));
        tts_list.set(curTTSKey, curTTSWs)

        // 连接建立完毕，读取数据进行识别
        curTTSWs.on('open', () => {
            devLog && console.log("-> 讯飞 tts服务连接成功！")
            send()
        })

        curTTSWs.on('message', (data, err) => {
            const { tts_list } = G_devices.get(device_id);

            if (err) {
                console.log('tts message error: ' + err)
                return
            }

            let res = JSON.parse(data)

            if (res.code != 0) {
                tts_list.delete(curTTSKey)
                devLog && console.log(`tts错误 ${res.code}: ${res.message}`)
                curTTSWs.close()
                resolve(false);
                return
            }

            const audio = res.data.audio;
            if (!audio) {
                tts_list.delete(curTTSKey)
                console.log(`tts错误：未返回音频流`)
                resolve(false);
                return
            }
            let audioBuf = Buffer.from(audio, 'base64')
            cb({
                // 根据服务控制
                is_over: res.code == 0 && res.data.status == 2,
                audio: audioBuf,

                // 固定写法
                TTS_resolve: resolve,
                curTTSWs,
                curTTSKey,
                device_id,
                reRecord,
            });

        })

        // // 资源释放
        // curTTSWs.on('close', () => {
        //     // 不等待 close 事件，直接释放资源 
        // })

        // 连接错误
        curTTSWs.on('error', (err) => {
            tts_list.delete(curTTSKey)
            console.log("websocket connect err: " + err)
            resolve(false);
        })
        // 传输数据
        function send() {
            const business = {
                "aue": "raw",
                "auf": "audio/L16;rate=16000",
                "vcn": "aisbabyxu",
                "tte": "UTF8",
                volume: 80
            }
            const frame = {
                // 填充common
                "common": {
                    "app_id": config.appid
                },
                // 填充business
                "business": tts_params_set ? tts_params_set(business) : business,
                // 填充data
                "data": {
                    "text": Buffer.from(text).toString('base64'),
                    "status": 2
                }
            }
            curTTSWs && curTTSWs.send(JSON.stringify(frame))
        }

    })
}

module.exports = TTS_FN;