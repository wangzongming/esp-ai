/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */

const https = require('https');

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
        api_key: api_key[llm_server].apiKey,
        llm: api_key[llm_server].llm,
    }

    const texts = {
        all_text: "",
        count_text: "",
    }

    const url = 'https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation';
    const headers = {
        'Content-Type': 'application/json',
        'Authorization': 'Bearer ' + config['api_key'],
        'X-DashScope-SSE': 'enable'
    };
    devLog && console.log("对话记录：\n", llm_historys)

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
            httpResponse += chunk.toString();
            const text_pattern = /"content":"(.*?)","role"/g;
            let match;
            while (match = text_pattern.exec(chunk)) {
                const chunk_text = match[1];
                if (chunk_text) {
                    devLog && console.log('LLM 输出 ：', chunk_text);
                    texts["count_text"] += chunk_text;
                    cb(device_id, { text, texts })
                }
            }
        });

        res.on('end', () => {
            cb(device_id, {
                text,
                is_over: true,
                texts
            })

            devLog && console.log('\n===\n', httpResponse, '\n===\n')
            devLog && console.log('\n===\n', texts["all_text"], '\n===\n')
            devLog && console.log('LLM connect close!\n')
        });
    });

    req.on('error', (e) => {
        console.log("llm connect err: " + err)
    });

    // 写入请求体
    req.write(body);
    req.end();

}

module.exports = LLM_FN;