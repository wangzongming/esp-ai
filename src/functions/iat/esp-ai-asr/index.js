
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

/**
 * 讯飞语音识别  
 * @param {String}      device_id           设备ID  
 * @param {String}      session_id          会话ID   
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Object}      iat_config          用户配置的 apikey 等信息   
 * @param {String}      iat_server          用户配置的 iat 服务 
 * @param {String}      llm_server          用户配置的 llm 服务 
 * @param {String}      tts_server          用户配置的 tts 服务 
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> {} })
 * @param {Function}    iatServerErrorCb    与 TTS 服务之间发生错误时调用，并且传入错误说明，eg: ttsServerErrorCb("意外错误") 
 * @param {Function}    cb                  IAT 识别的结果调用这个方法回传给框架 eg: cb({ text: "我是语音识别结果"  })
 * @param {Function}    logSendAudio        记录发送音频数据给服务的函数，框架在合适的情况下会进行调用
 * @param {Function}    connectServerBeforeCb 连接 iat 服务逻辑开始前需要调用这个方法告诉框架：eg: connectServerBeforeCb()
 * @param {Function}    connectServerCb     连接 iat 服务后需要调用这个方法告诉框架：eg: connectServerCb(true)
 * @param {Function}    serverTimeOutCb     当 IAT 服务连接成功了，但是长时间不响应时
 * @param {Function}    iatEndQueueCb       iat 静默时间达到后触发， 一般在这里面进行最后一帧的发送，告诉服务端结束识别 
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.iat_info("iat 专属信息")
 * 
 *  
*/
function IAT_FN({ device_id, session_id, log, devLog, iat_config, iat_server, llm_server, tts_server, cb, iatServerErrorCb, logWSServer, logSendAudio, connectServerCb, connectServerBeforeCb, serverTimeOutCb, iatEndQueueCb }) {
    try {
        const { api_key, vad_first = '', vad_course = '', ...other_config } = iat_config;
        if (!api_key) return log.error(`请配给 IAT 配置 api_key 参数。`)

        // 如果关闭后 message 还没有被关闭，需要定义一个标志控制
        let shouldClose = false;
        // 这个标志必须设置
        let iat_server_connected = false;

        connectServerBeforeCb();
        const iat_ws = new WebSocket(`wss://espai.natapp4.cc/v1/asr?api_key=${api_key}&vad_first=${vad_first}&vad_course=${vad_course}`);

        logWSServer({
            close: () => {
                shouldClose = true;
                iat_server_connected = false;
                iat_ws.close()
            },
            end: ()=>{
                iat_ws.send("end");  
            }
        });

        // 连接建立完毕，读取数据进行识别
        iat_ws.on('open', (event) => {
            if (shouldClose) return;
            devLog && log.iat_info("-> ESP-AI ASR 服务连接成功: " + session_id)
            iat_server_connected = true;
            connectServerCb(true);
        })

        // 当达到静默时间后会自动执行这个任务
        iatEndQueueCb(() => {
            if (shouldClose) return;
            iat_ws.send("end"); 
        })

        let realStr = "";
        // 得到识别结果后进行处理，仅供参考，具体业务具体对待
        iat_ws.on('message', (event) => { 
            if (shouldClose) return;
            if (!event) return;
            const data = JSON.parse(event); 
            switch (data.type) {
                case 'result':
                    realStr += data.text;
                    devLog && log.iat_info(data.text)
                    break;
                case 'final': 
                    if (shouldClose) return;
                    realStr += data.text;
                    devLog && log.iat_info(`IAT 已完成：${realStr}`)
                    cb({ text: realStr, device_id });
                    connectServerCb(false);
                    shouldClose = true;
                    iat_server_connected = false;
                    break; 
                case 'error': 
                    if (shouldClose) return;
                    log.error(`ESP-AI-ASR 服务错误： ${data.text}`)
                    iatServerErrorCb(`ESP-AI-ASR 服务错误： ${data.text}`, data.code);
                    connectServerCb(false);
                    shouldClose = true;
                    iat_server_connected = false;
                    break;
            }
        })

        // 资源释放
        iat_ws.on('close', () => {
            if (shouldClose) return;
            devLog && log.iat_info("-> ESP-AI-ASR 服务已关闭：", session_id)
            connectServerCb(false);
        })

        // 建连错误
        iat_ws.on('error', (err) => {
            if (shouldClose) return;
            iatServerErrorCb(err);
            connectServerCb(false);
        })


        function send_pcm(data) {
            if (shouldClose) return;
            if (!iat_server_connected) return;
            iat_ws.send(data); 
        }

        logSendAudio(send_pcm)
    } catch (err) {
        connectServerCb(false);
        console.log(err);
        log.error("ESP-AI-ASR  插件错误：", err)
    }
}

module.exports = IAT_FN;