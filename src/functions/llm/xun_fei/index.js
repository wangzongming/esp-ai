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
const getServerURL = require("../../getServerURL");
const log = require("../../../utils/log");


/**
 * 大语言模型插件
 * @param {String}      device_id           设备id 
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Object}      llm_config          用户配置的 apikey 等信息   
 * @param {String}      iat_server          用户配置的 iat 服务 
 * @param {String}      llm_server          用户配置的 llm 服务 
 * @param {String}      tts_server          用户配置的 tts 服务 
 * @param {String}      text                对话文本
 * @param {Function}    connectServerBeforeCb 连接 LLM 服务逻辑开始前需要调用这个方法告诉框架：eg: connectServerBeforeCb()
 * @param {Function}    connectServerCb     连接 LLM 服务后需要调用这个方法告诉框架：eg: connectServerCb(true)
 * @param {Function}    cb                  LLM 服务返回音频数据时调用，eg: cb({  count_text, text, texts })
 * @param {Function}    llmServerErrorCb    与 LLM 服务之间发生错误时调用，并且传入错误说明，eg: llmServerErrorCb("意外错误")
 * @param {Function}    llm_params_set      用户配置的设置 LLM 参数的函数
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> { 中断逻辑... } })
 * @param {{role, content}[]}  llm_init_messages   用户配置的初始化时的对话数据
 * @param {{role, content}[]}  llm_historys        llm 历史对话数据
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.llm_info("llm 专属信息")
 *  
*/
function LLM_FN({ device_id, devLog, llm_config, iat_server, llm_server, tts_server, text, llmServerErrorCb, llm_init_messages = [], llm_historys = [], cb, llm_params_set, logWSServer, connectServerBeforeCb, connectServerCb }) {
    try {
        const { llm, appid, apiSecret, apiKey, ...other_config } = llm_config;
        if (!apiKey) return log.error(`请配给 LLM 配置 apiKey 参数。`)
        if (!apiSecret) return log.error(`请配给 LLM 配置 apiSecret 参数。`)
        if (!appid) return log.error(`请配给 LLM 配置 appid 参数。`)

        // 如果关闭后 message 还没有被关闭，需要定义一个标志控制
        let shouldClose = false;

        // 这个对象是固定写法，每个 TTS 都必须按这个结构定义
        const texts = {
            all_text: "",
            count_text: "",
        }
        connectServerBeforeCb();
        const llm_ws = new WebSocket(getServerURL("LLM", { appid, apiSecret, apiKey, llm, iat_server, llm_server, tts_server, }));
        logWSServer({
            close: () => {
                connectServerCb(false);
                shouldClose = true;
                llm_ws.close(1000, 'Normal closure');
            }
        })
        llm_ws.on('open', () => {
            connectServerCb(true);
            devLog && log.llm_info("-> llm 服务连接成功！")
            texts["all_text"] = "";
            texts["count_text"] = "";
            send();
        })

        llm_ws.on('message', async (resultData, err) => {
            if (err) {
                log.llm_info('tts message error: ' + err)
                return
            }
            if (shouldClose) return;

            const jsonData = JSON.parse(resultData)
            if (jsonData.header.code !== 0) {
                llmServerErrorCb(device_id, "LLM 数据返回错误！")
                log.llm_info('提问失败: ', JSON.stringify(jsonData))
                connectServerCb(false);
                return
            }
            const chunk_text = jsonData.payload.choices.text[0]["content"];
            devLog === 2 && log.llm_info('LLM 输出 ：', chunk_text);
            // log.llm_info('LLM 输出 ：', chunk_text);
            texts["count_text"] += chunk_text;

            cb({
                text,
                is_over: jsonData.header.code === 0 && jsonData.header.status === 2,
                texts,
                shouldClose,
                chunk_text: chunk_text 
            })

        })

        llm_ws.on('close', () => {
            if (shouldClose) return;
            // devLog && log.llm_info('\n===\n', texts["all_text"], '\n===\n')
            devLog && log.llm_info('LLM connect close!\n')
            connectServerCb(false);
        })

        llm_ws.on('error', (err) => {
            llmServerErrorCb("llm websocket connect err: " + err)
            connectServerCb(false);
        })

        function send() {
            // devLog && log.llm_info("对话记录：\n")
            // devLog && console.log(llm_historys);
            const frame = {
                "header": {
                    "app_id": appid,
                    "uid": device_id
                },
                "parameter": {
                    "chat": {
                        "temperature": 0.5,
                        "max_tokens": 500,
                        ...other_config,
                        "domain": {
                            "v4.0": "4.0Ultra",
                            "v3.5": "generalv3.5",
                            "v3.1": "generalv3",
                            // "v2.1": "generalv2",
                            "v1.1": "lite",
                            "max-32k": "max-32k",
                            "pro-128k": "pro-128k",
                        }[llm],
                    }
                },
                "payload": {
                    "message": {
                        "text": [
                            ...llm_init_messages,
                            ...llm_historys,
                            {
                                "role": "user", "content": text
                            },
                        ]
                    }
                }
            }
            llm_ws.send(JSON.stringify(llm_params_set ? llm_params_set({...frame}) : frame))
        }
    } catch (err) {
        console.log("讯飞 LLM 插件错误：",err);
        log.error("讯飞 LLM 插件错误：", err)
        connectServerCb(false);
    }

}

module.exports = LLM_FN;