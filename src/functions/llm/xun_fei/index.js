/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws') 
const getServerURL = require("../../getServerURL");


/**
 * 大语言模型
 * @param {String} device_id 设备id 
 * @param {String} text 对话文本
*/
function LLM_FN(device_id, { text, onError, cb }) {
    const { devLog, api_key, llm_server, llm_init_messages = [], llm_params_set } = G_config;
    devLog && console.log('\n\n=== 开始请求 LLM，输入: ', text, " ===");
    const { llm_historys = [] } = G_devices.get(device_id);
    const config = {
        appid: api_key[llm_server].appid,
    }

    const texts = {
        all_text: "",
        count_text: "",
    }

    const llm_ws = new WebSocket(getServerURL("LLM"));
    G_devices.set(device_id, {
        ...G_devices.get(device_id),
        llm_ws,
    })
    llm_ws.on('open', () => {
        devLog && console.log("-> llm 服务连接成功！")
        texts["all_text"] = "";
        texts["count_text"] = "";
        send();
    })

    llm_ws.on('message', async (resultData, err) => {
        if (err) {
            console.log('tts message error: ' + err)
            return
        }

        const jsonData = JSON.parse(resultData)
        if (jsonData.header.code !== 0) {
            onError(device_id, { text: "LLM 数据返回错误！" })
            console.log('提问失败: ', jsonData)
            return
        }
        const chunk_text = jsonData.payload.choices.text[0]["content"];
        devLog && console.log('LLM 输出 ：', chunk_text);
        texts["count_text"] += chunk_text;

        cb(device_id, {
            text,
            is_over: jsonData.header.code === 0 && jsonData.header.status === 2,
            texts
        })

    })

    llm_ws.on('close', () => {
        devLog && console.log('\n===\n', texts["all_text"], '\n===\n')
        devLog && console.log('LLM connect close!\n')
    })

    llm_ws.on('error', (err) => {
        console.log("llm websocket connect err: " + err)
    })

    function send() {
        devLog && console.log("对话记录：\n", llm_historys)
        const frame = {
            "header": {
                "app_id": config.appid,
                "uid": device_id
            },
            "parameter": {
                "chat": {
                    "domain": "generalv3.5",
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