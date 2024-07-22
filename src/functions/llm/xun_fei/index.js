/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const getServerURL = require("../../getServerURL");
const log = require("../../../utils/log");


/**
 * 大语言模型插件
 * @param {String}      device_id           设备id 
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Object}      api_key             用户配置的key   
 * @param {String}      text                对话文本
 * @param {Function}    cb                  LLM 服务返回音频数据时调用，eg: cb({ text, texts })
 * @param {Function}    llmServerErrorCb    与 LLM 服务之间发生错误时调用，并且传入错误说明，eg: llmServerErrorCb("意外错误")
 * @param {Function}    llm_params_set      用户配置的设置 LLM 参数的函数
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> {} })
 * @param {{role, content}[]}  llm_init_messages   用户配置的初始化时的对话数据
 * @param {{role, content}[]}  llm_historys        llm 历史对话数据
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.llm_info("llm 专属信息")
 *  
*/
function LLM_FN({ device_id, devLog, api_key, text, llmServerErrorCb, llm_init_messages = [], llm_historys = [], cb, llm_params_set, logWSServer }) {
    const config = { ...api_key }

    // 这个对象是固定写法，每个 TTS 都必须按这个结构定义
    const texts = {
        all_text: "",
        count_text: "",
    }

    const llm_ws = new WebSocket(getServerURL("LLM"));
    logWSServer(llm_ws)
    llm_ws.on('open', () => {
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

        const jsonData = JSON.parse(resultData)
        if (jsonData.header.code !== 0) {
            llmServerErrorCb(device_id, "LLM 数据返回错误！")
            log.llm_info('提问失败: ', JSON.stringify(jsonData))
            return
        }
        const chunk_text = jsonData.payload.choices.text[0]["content"];
        devLog && log.llm_info('LLM 输出 ：', chunk_text);
        texts["count_text"] += chunk_text;

        cb({
            text,
            is_over: jsonData.header.code === 0 && jsonData.header.status === 2,
            texts
        })

    })

    llm_ws.on('close', () => {
        // devLog && log.llm_info('\n===\n', texts["all_text"], '\n===\n')
        devLog && log.llm_info('LLM connect close!\n')
    })

    llm_ws.on('error', (err) => {
        llmServerErrorCb("llm websocket connect err: " + err)
    })

    function send() {
        devLog && log.llm_info("对话记录：\n")
        devLog && console.log(llm_historys);
        const frame = {
            "header": {
                "app_id": config.appid,
                "uid": device_id
            },
            "parameter": {
                "chat": {
                    "domain": {
                        "v4.0": "4.0Ultra",
                        "v3.5": "generalv3.5",
                        "v3.1": "generalv3",
                        "v2.1": "generalv2",
                        "v1.1": "general",
                    }[config.llm],
                    "temperature": 0.5,
                    "max_tokens": 500
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
        llm_ws.send(JSON.stringify(llm_params_set ? llm_params_set(frame) : frame))
    }
}

module.exports = LLM_FN;