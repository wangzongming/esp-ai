/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */

const https = require('https');
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
 * @param {{role, content}[]}  llm_historys llm 历史对话数据
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.llm_info("llm 专属信息")
 *  
*/
function LLM_FN({ devLog, api_key, text, llmServerErrorCb, llm_init_messages = [], llm_historys = [], cb, llm_params_set, logWSServer }) {
    const config = { ...api_key }

    // 这个对象是固定写法，每个 TTS 都必须按这个结构定义
    const texts = {
        all_text: "",
        count_text: "",
    }

    const url = 'https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation';
    const headers = {
        'Content-Type': 'application/json',
        'Authorization': 'Bearer ' + config['apiKey'],
        'X-DashScope-SSE': 'enable'
    };
    devLog && log.llm_info("对话记录：\n", llm_historys)

    const r_params = {
        "model": config.llm,
        "input": {
            "messages": [
                ...llm_init_messages,
                ...llm_historys,
                {
                    "role": "user", "content": text
                },
            ]
        },
        "parameters": {
            "incremental_output": true,
            "result_format": "message"
        }
    };
    const body = JSON.stringify(llm_params_set ? llm_params_set(r_params) : r_params);

    const options = {
        method: 'POST',
        headers: headers
    };
    const req = https.request(url, options, (res) => {
        let httpResponse = '';

        res.on('data', (chunk) => {
            try {
                const chunk_obj = JSON.parse(chunk.toString());
                if (chunk_obj.code) {
                    llmServerErrorCb("积灵 LLM 报错: " + chunk.toString())
                }
            } catch (err) { }
            httpResponse += chunk.toString();
            const text_pattern = /"content":"(.*?)","role"/g;
            let match;
            while (match = text_pattern.exec(chunk)) {
                const chunk_text = match[1];
                if (chunk_text) {
                    devLog && log.llm_info('LLM 输出 ：', chunk_text);
                    texts["count_text"] += chunk_text;
                    cb({ text, texts })
                }
            }
        });

        res.on('end', () => {
            cb({
                text,
                is_over: true,
                texts
            })

            // devLog && log.llm_info('\n===\n', httpResponse, '\n===\n')
            devLog && log.llm_info('===')
            devLog && log.llm_info(texts["all_text"])
            devLog && log.llm_info('===')
            devLog && log.llm_info('LLM connect close!\n')
        });
    });

    req.on('error', (err) => {
        llmServerErrorCb("llm connect err: " + err)
    });

    // 写入请求体
    req.write(body);
    req.end();

}

module.exports = LLM_FN;