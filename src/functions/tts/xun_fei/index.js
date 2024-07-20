/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const getServerURL = require("../../getServerURL"); 

/**
 * TTS 插件封装 - 讯飞 TTS
 * @param {String}      device_id           设备ID   
 * @param {String}      text                待播报的文本   
 * @param {Object}      api_key             用户配置的key   
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Function}    tts_params_set      用户自定义传输给 TTS 服务的参数，eg: tts_params_set(参数体)
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> {} })
 * @param {Function}    ttsServerErrorCb    与 TTS 服务之间发生错误时调用，并且传入错误说明，eg: ttsServerErrorCb("意外错误")
 * @param {Function}    cb                  TTS 服务返回音频数据时调用，eg: cb({ audio: 音频base64, ... })
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.tts_info("tts 专属信息")
*/
function TTS_FN({ text, devLog, api_key, logWSServer, tts_params_set, cb, log, ttsServerErrorCb }) {
    const config = { ...api_key }

    return new Promise((resolve) => {
        const ws = new WebSocket(getServerURL("TTS"));

        // 如果 ws 服务是个 WebSocket 对象，请调用这个方法。框架在适合的时候会调用 .close() 方法
        logWSServer(ws)

        // 连接建立完毕，读取数据进行识别
        ws.on('open', () => {
            devLog && log.tts_info("-> 讯飞 TTS 服务连接成功！")
            send()
        })

        ws.on('message', (data, err) => {
            if (err) {
                log.tts_info('tts message error: ' + err)
                return
            }

            let res = JSON.parse(data)

            if (res.code != 0) {
                ttsServerErrorCb(`tts错误 ${res.code}: ${res.message}`)
                ws.close()
                resolve(false);
                return
            }

            const audio = res.data.audio;
            if (!audio) {
                ttsServerErrorCb(`tts错误：未返回音频流`)
                resolve(false);
                return
            }
            let audioBuf = Buffer.from(audio, 'base64')
            cb({
                // 根据服务控制
                is_over: res.code == 0 && res.data.status == 2,
                audio: audioBuf,

                // 固定写法
                resolve: resolve,
                ws: ws
            });

        })

        // 资源释放, 某些服务需要在这里面调用一次 cb({ is_over: true })
        // ws.on('close', () => { })

        // 连接错误
        ws.on('error', (err) => {
            ttsServerErrorCb(`tts错误："websocket connect err: ${err}`)
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
            ws && ws.send(JSON.stringify(frame))
        }

    })
}

module.exports = TTS_FN;