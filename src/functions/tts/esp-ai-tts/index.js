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
const fs = require('fs')
const axios = require('axios');
const { PassThrough } = require('stream');

/**
 * TTS 插件封装 - 讯飞 TTS
 * @param {String}      device_id           设备ID   
 * @param {String}      text                待播报的文本   
 * @param {Object}      tts_config          用户配置的 apikey 等信息 
 * @param {String}      iat_server          用户配置的 iat 服务 
 * @param {String}      llm_server          用户配置的 llm 服务 
 * @param {String}      tts_server          用户配置的 tts 服务 
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Function}    tts_params_set      用户自定义传输给 TTS 服务的参数，eg: tts_params_set(参数体)
 * @param {Function}    connectServerBeforeCb 连接 tts 服务逻辑开始前需要调用这个方法告诉框架：eg: connectServerBeforeCb()
 * @param {Function}    connectServerCb     连接 tts 服务后需要调用这个方法告诉框架：eg: connectServerCb(true)
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> {  中断逻辑...  } })
 * @param {Function}    ttsServerErrorCb    与 TTS 服务之间发生错误时调用，并且传入错误说明，eg: ttsServerErrorCb("意外错误")
 * @param {Function}    cb                  TTS 服务返回音频数据时调用，eg: cb({ audio: 音频base64, ... })
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.tts_info("tts 专属信息")
*/
function TTS_FN({ text, devLog, tts_config, logWSServer, tts_params_set, cb, log, ttsServerErrorCb, connectServerCb, connectServerBeforeCb }) {
    try {
        const { url = 'https://api.espai2.fun/ai_api/tts', reference_id = "cosyvoice-v2-espai-ad258e829c08424db6ce59ac7a38b417", api_key, ...other_config } = tts_config;
        if (!api_key) return log.error(`请配给 TTS 配置 api_key 参数。`)
        if (!url) return log.error(`请配给 TTS 配置 url 参数。`);
        let shouldClose = false;
        /**
         * ESP-AI TTS Server 请求接口封装
         * 服务返回 wav 流，需要转为 mp3 流
        */
        async function sendTTSRequest({ url, streaming, ...other_config }) {
            const data = { ...other_config, streaming, api_key };

            try {
                connectServerBeforeCb(); 
                const response = await axios.post(url, {
                    ...data,
                    text: text,
                    api_key: api_key
                }, {
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    responseType: 'stream'
                });
                const ws = {
                    close() {
                        shouldClose = true;
                    }
                };
                logWSServer(ws)

                connectServerCb(true);
                const audioStream = response.data;
                const stream = new PassThrough();
                audioStream.pipe(stream);
                stream.on('data', (chunk) => {
                    cb({ is_over: false, audio: chunk, ws });
                })
                stream.on('end', () => {
                    cb({ is_over: true, audio: "", ws });
                    connectServerCb(false);
                });
 
            } catch (error) {
                console.error(`tts错误 ${error.message}`);
                console.error(`Status: ${error.response?.status}`);
                connectServerCb(false);
            }
        }

        sendTTSRequest({ ...other_config, url, text, reference_id, streaming: true, reference_audio: [] });
    } catch (err) {
        connectServerCb(false);
        log.error(`ESP-AI TTS 错误： ${err}`)
    }
}

module.exports = TTS_FN;