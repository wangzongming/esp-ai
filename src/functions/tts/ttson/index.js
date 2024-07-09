/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const { PassThrough } = require('stream');
const https = require('https');

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
        token: api_key[tts_server].token,
    }

    const url = `https://u95167-bd74-2aef8085.westx.seetacloud.com:8443/flashsummary/tts?token=${config.token}`;
    let language = "ZH";

    if (/[a-zA-Z]/.test(text)) {
        language = "auto";
    }

    const _payload = {
        voice_id: 430,
        text: text,
        format: "wav",
        to_lang: language,
        auto_translate: 0,
        voice_speed: "0%",
        speed_factor: 1,
        rate: "1.0",
        client_ip: "ACGN",
        emotion: 1
    }
    const payload = JSON.stringify(tts_params_set ? tts_params_set(_payload) : _payload);
    const options = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36'
        }
    };

    const getAR = () => {
        return new Promise((resolve, reject) => {
            const req = https.request(url, options, (res) => {
                let data = '';

                res.on('data', (chunk) => {
                    data += chunk;
                });

                res.on('end', () => {
                    if (res.statusCode !== 200) {
                        console.error(`Error: ${res.statusCode}`);
                        reject(null);
                    } else {
                        const responseJson = JSON.parse(data);
                        resolve(`${responseJson.url}:${responseJson.port}/flashsummary/retrieveFileData?stream=True&token=${config.token}&voice_audio_path=${responseJson.voice_path}`);
                    }
                });
            });
            req.on('error', (e) => {
                console.error(`Error fetching audio URL: ${e.message}`);
                reject(null);
            });

            req.write(payload);
            req.end();
        })
    }


    return new Promise(async (resolve) => {
        devLog && console.log('\n=== 开始请求TTS: ', text, " ===");
        const ar = await getAR();

        if (ar) {
            devLog && console.log("音频地址：", ar);

            const { ws: ws_client, tts_list } = G_devices.get(device_id);
            // 开始播放直接先让 esp32 暂停采集音频，不然处理不过来
            if (pauseInputAudio) {
                ws_client && ws_client.send("pause_voice");
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    client_out_audio_ing: true,
                })
            }
            const wavStream = wavUrlToStream(ar);
            const curTTSKey = tts_list.size;
            const curTTSWs = wavStream;
            tts_list.set(curTTSKey, curTTSWs)

            devLog && console.log("-> tts服务连接成功！")
            wavStream.on('data', (chunk) => {
                console.log(`Received ${chunk.length} bytes of data.`);
                //     let audioBuf = Buffer.from(audio, 'base64')
                cb({
                    // 根据服务控制
                    is_over: false,
                    audio: chunk,

                    // 固定写法
                    TTS_resolve: resolve,
                    curTTSWs,
                    curTTSKey,
                    device_id,
                    reRecord,
                });
            });
            wavStream.on('end', () => {
                console.log('No more data.');
                cb({
                    // 根据服务控制
                    is_over: true,
                    audio: "", 
                    // 固定写法
                    TTS_resolve: resolve,
                    curTTSWs,
                    curTTSKey,
                    device_id,
                    reRecord,
                });
            });
            wavStream.on('error', (err) => {
                console.error(`Stream error: ${err.message}`);
            });

        } else {
            devLog && console.log(`tts错误 ${res.code}: ${res.message}`)
            curTTSWs.close()
            resolve(false);

        }



        // const { ws: ws_client, tts_list } = G_devices.get(device_id);
        // // 开始播放直接先让 esp32 暂停采集音频，不然处理不过来
        // if (pauseInputAudio) {
        //     ws_client && ws_client.send("pause_voice");
        //     G_devices.set(device_id, {
        //         ...G_devices.get(device_id),
        //         client_out_audio_ing: true,
        //     })
        // }

        // const curTTSKey = tts_list.size;
        // const curTTSWs = new WebSocket(getServerURL("TTS"));
        // tts_list.set(curTTSKey, curTTSWs)

        // // 连接建立完毕，读取数据进行识别
        // curTTSWs.on('open', () => {
        //     devLog && console.log("-> 讯飞 tts服务连接成功！")
        //     send()
        // })

        // curTTSWs.on('message', (data, err) => {
        //     const { tts_list } = G_devices.get(device_id);

        //     if (err) {
        //         console.log('tts message error: ' + err)
        //         return
        //     }

        //     let res = JSON.parse(data)

        //     if (res.code != 0) {
        //         tts_list.delete(curTTSKey)
        //         devLog && console.log(`tts错误 ${res.code}: ${res.message}`)
        //         curTTSWs.close()
        //         resolve(false);
        //         return
        //     }

        //     const audio = res.data.audio;
        //     if (!audio) {
        //         tts_list.delete(curTTSKey)
        //         console.log(`tts错误：未返回音频流`)
        //         resolve(false);
        //         return
        //     }
        //     let audioBuf = Buffer.from(audio, 'base64')
        //     cb({
        //         // 根据服务控制
        //         is_over: res.code == 0 && res.data.status == 2,
        //         audio: audioBuf,

        //         // 固定写法
        //         TTS_resolve: resolve,
        //         curTTSWs,
        //         curTTSKey,
        //         device_id,
        //         reRecord,
        //     });

        // }) 

        // // 连接错误
        // curTTSWs.on('error', (err) => {
        //     tts_list.delete(curTTSKey)
        //     console.log("websocket connect err: " + err)
        //     resolve(false);
        // })
        // // 传输数据
        // function send() {
        //     const business = {
        //         "aue": "raw",
        //         "auf": "audio/L16;rate=16000",
        //         "vcn": "aisbabyxu",
        //         "tte": "UTF8",
        //         volume: 80
        //     }
        //     const frame = {
        //         // 填充common
        //         "common": {
        //             "app_id": config.appid
        //         },
        //         // 填充business
        //         "business": tts_params_set ? tts_params_set(business) : business,
        //         // 填充data
        //         "data": {
        //             "text": Buffer.from(text).toString('base64'),
        //             "status": 2
        //         }
        //     }
        //     curTTSWs && curTTSWs.send(JSON.stringify(frame))
        // }

    })
}
function wavUrlToStream(url) {
    const stream = new PassThrough();

    https.get(url, (response) => {
        if (response.statusCode !== 200) {
            stream.emit('error', new Error(`Request failed with status code ${response.statusCode}`));
            return;
        }

        response.pipe(stream);
    }).on('error', (err) => {
        stream.emit('error', err);
    });

    return stream;
}
module.exports = TTS_FN;