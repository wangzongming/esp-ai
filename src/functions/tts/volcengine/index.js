/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const zlib = require('zlib');
const { v4: uuidv4 } = require('uuid');

// 本文件对接文档： https://www.volcengine.com/docs/6561/79823

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
    if(! api_key[tts_server].tts){
        console.log("您设置使用火山TTS，但是没有按照稳定没有配置tts服务");
        return;
    }
    const config = {
        appid: api_key[tts_server].tts.appid,
        accessToken: api_key[tts_server].tts.accessToken,
    }   
    console.log(config)
    const host = "openspeech.bytedance.com";
    const api_url = `wss://${host}/api/v1/tts/ws_binary`;
    const default_header = Buffer.from([0x11, 0x10, 0x11, 0x00]);
    const audio_config = {
        voice_type: "BV001_streaming",
        encoding: "pcm",
        rate: 16000, // 目前只支持16k
        speed_ratio: 1.0,
        // volume_ratio: 1.6,
        pitch_ratio: 1.0,
    }
    const request_json = {
        app: {
            appid: config["appid"],
            token: config["accessToken"],
            cluster: "volcano_tts"
        },
        user: {
            uid: device_id
        },
        audio: tts_params_set ? tts_params_set(audio_config) : audio_config,
        request: {
            reqid: uuidv4(),
            text: text,
            text_type: "plain",
            operation: "submit"
        }
    };


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

        const submit_request_json = JSON.parse(JSON.stringify(request_json));
        let payload_bytes = Buffer.from(JSON.stringify(submit_request_json));
        payload_bytes = zlib.gzipSync(payload_bytes);  // if no compression, comment this line
        const full_client_request = Buffer.concat([default_header, Buffer.alloc(4), payload_bytes]);
        full_client_request.writeUInt32BE(payload_bytes.length, 4);
        const curTTSWs = new WebSocket(api_url, { headers: { "Authorization": `Bearer; ${config["accessToken"]}` }, perMessageDeflate: false });

        const curTTSKey = tts_list.size;
        tts_list.set(curTTSKey, curTTSWs)

        // 连接建立完毕，读取数据进行识别
        curTTSWs.on('open', () => {
            devLog && console.log("-> 讯飞 tts服务连接成功！")
            send()
        })

        curTTSWs.on('message', (res, err) => {
            const { tts_list } = G_devices.get(device_id);
            if (err) {
                console.log('tts message error: ' + err)
                return
            } 
            // const protocol_version = res[0] >> 4;
            const header_size = res[0] & 0x0f;
            const message_type = res[1] >> 4;
            const message_type_specific_flags = res[1] & 0x0f;
            // const serialization_method = res[2] >> 4;
            const message_compression = res[2] & 0x0f; 
            let payload = res.slice(header_size * 4);
            let done = false;
            if (message_type === 0xb) {  // audio-only server response
                if (message_type_specific_flags === 0) {  // no sequence number as ACK
                    console.log("                Payload size: 0");
                    return false;
                } else {
                    const sequence_number = payload.readInt32BE(0); 
                    payload = payload.slice(8); 

                    done = sequence_number < 0;
                }
            } else if (message_type === 0xf) {
                const code = payload.readUInt32BE(0);
                const msg_size = payload.readUInt32BE(4);
                let error_msg = payload.slice(8);
                if (message_compression === 1) {
                    error_msg = zlib.gunzipSync(error_msg);
                }
                error_msg = error_msg.toString('utf-8');
                console.log(`          Error message code: ${code}`);
                console.log(`          Error message size: ${msg_size} bytes`);
                console.log(`                  Error data: ${JSON.stringify(request_json, null, 4)}`); 
                console.log(`               Error message: ${error_msg}`);
                
                tts_list.delete(curTTSKey)
                devLog && console.log(`tts错误 ${res.code}: ${res.message}`)
                curTTSWs.close()
                resolve(false);
                return 
            } else if (message_type === 0xc) { 
                payload = payload.slice(4);
                if (message_compression === 1) {
                    payload = zlib.gunzipSync(payload);
                }
                console.log(`            Frontend message: ${payload}`);
            } else {
                console.log("undefined message type!");
                done = true;
            }
 
            cb({
                // 根据服务控制
                is_over: done,
                audio: payload,

                // 固定写法
                TTS_resolve: resolve,
                curTTSWs,
                curTTSKey,
                device_id,
                reRecord,
            });

 
        })

        // 连接错误
        curTTSWs.on('error', (err) => {
            tts_list.delete(curTTSKey)
            console.log("websocket connect err: " + err)
            resolve(false);
        })
        // 传输数据
        function send() {
            curTTSWs && curTTSWs.send(full_client_request);
        }

    })
}

module.exports = TTS_FN;