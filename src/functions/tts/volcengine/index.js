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
const zlib = require('zlib');
const { v4: uuidv4 } = require('uuid');

// 本文件对接文档： https://www.volcengine.com/docs/6561/79823


/**
 * TTS 插件封装 - 火山引擎 TTS 
 * @param {String}      device_id           设备ID   
 * @param {String}      text                待播报的文本   
 * @param {Object}      tts_config          用户配置的 apikey 等信息    
 * @param {String}      iat_server          用户配置的 iat 服务 
 * @param {String}      llm_server          用户配置的 llm 服务 
 * @param {String}      tts_server          用户配置的 tts 服务 
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Function}    connectServerBeforeCb 连接 tts 服务逻辑开始前需要调用这个方法告诉框架：eg: connectServerBeforeCb()
 * @param {Function}    connectServerCb     连接 tts 服务后需要调用这个方法告诉框架：eg: connectServerCb(true)
 * @param {Function}    tts_params_set      用户自定义传输给 TTS 服务的参数，eg: tts_params_set(参数体)
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> { 中断逻辑...  }  })
 * @param {Function}    ttsServerErrorCb    与 TTS 服务之间发生错误时调用，并且传入错误说明，eg: ttsServerErrorCb("意外错误")
 * @param {Function}    cb                  TTS 服务返回音频数据时调用，eg: cb({ audio: 音频base64, ... })
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.tts_info("tts 专属信息")
*/
function TTS_FN({ device_id, text, devLog, tts_config, logWSServer, tts_params_set, cb, log, ttsServerErrorCb, connectServerCb, connectServerBeforeCb }) {
    try {
        const { appid, accessToken, appConfig = {}, is_clone, ...other_config } = tts_config;
        if (!accessToken) return log.error(`请配给 TTS 配置 accessToken 参数。`)
        if (!appid) return log.error(`请配给 TTS 配置 appid 参数。`)

        const host = "openspeech.bytedance.com";
        const api_url = `wss://${host}/api/v1/tts/ws_binary`;
        const default_header = Buffer.from([0x11, 0x10, 0x11, 0x00]);
        const audio_config = {
            voice_type: "BV001_streaming",
            rate: 16000,
            speed_ratio: 1.0,
            pitch_ratio: 1.0,
            ...other_config,
            encoding: "mp3",
        }

        // 复刻的音色有些参数不一样
        if (is_clone) {
            audio_config["rate"] = 24000; // 大模型复刻必须 24k
            appConfig.cluster = "volcano_icl"
        }


        const request_json = {
            app: {
                cluster: "volcano_tts",
                ...appConfig,
                appid: appid,
                token: accessToken,
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


        const submit_request_json = JSON.parse(JSON.stringify(request_json));
        let payload_bytes = Buffer.from(JSON.stringify(submit_request_json));
        payload_bytes = zlib.gzipSync(payload_bytes);  // if no compression, comment this line
        const full_client_request = Buffer.concat([default_header, Buffer.alloc(4), payload_bytes]);
        full_client_request.writeUInt32BE(payload_bytes.length, 4);

        connectServerBeforeCb();
        const curTTSWs = new WebSocket(api_url, { headers: { "Authorization": `Bearer; ${accessToken}` }, perMessageDeflate: false });
        logWSServer({
            close(){ 
                curTTSWs.OPEN && curTTSWs.close();
            }
        })

        // 连接建立完毕，读取数据进行识别
        curTTSWs.on('open', () => {
            connectServerCb(true);
            devLog && log.tts_info("-> 火山引擎 TTS 服务连接成功！")
            send()
        })

        curTTSWs.on('message', (res, err) => {
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
                    // console.log("                Payload size: 0");
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

                log.tts_info(`发送字符串：${text}`);
                ttsServerErrorCb(`火山 TTS 接口返回错误 ${res.code}: ${res.message} ${error_msg}`)
                curTTSWs.close()
                connectServerCb(false);
                cb({ is_over: true, audio: "", ws: curTTSWs });
                return
            } else if (message_type === 0xc) {
                payload = payload.slice(4);
                if (message_compression === 1) {
                    payload = zlib.gunzipSync(payload);
                }
                log.tts_info(`            Frontend message: ${payload}`);
            } else {
                log.tts_info("undefined message type!");
                done = true;
            }

            cb({
                // 根据服务控制
                is_over: done,
                audio: payload,

                ws: curTTSWs,
            });


        })

        // 连接错误
        curTTSWs.on('error', (err) => {
            ttsServerErrorCb("火山 TTS 连接错误: " + err)
            connectServerCb(false);
        })
        // 传输数据
        function send() {
            curTTSWs && curTTSWs.send(full_client_request);
        }
    } catch (err) {
        connectServerCb(false);
        log.error(`火山 TTS 错误： ${err}`)
    }
}

module.exports = TTS_FN;