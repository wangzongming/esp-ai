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
        llm: api_key[llm_server].llm || 'gpt-3.5-turbo',
    };

    const texts = {
        all_text: "",
        count_text: "",
    };

    const url = 'https://api.xn--5kv132d.com/v1/chat/completions';
    const headers = {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${config.api_key}`,
    };
    devLog && console.log("对话记录：\n", llm_historys);

    const r_params = {
        model: config.llm,
        messages: [
            ...llm_init_messages,
            ...llm_historys,
            { role: "user", content: text },
        ],
        stream: true, // 启用流式输出
    };
    const body = JSON.stringify(llm_params_set ? llm_params_set(r_params) : r_params);

    const options = {
        method: 'POST',
        headers: headers,
    };

    const req = https.request(url, options, (res) => {
        res.on('data', (chunk) => {
            const data = chunk.toString().split('\n').filter(line => line.trim() !== '');
            
            for (const line of data) {
                if (line.trim() === 'data: [DONE]') {
                    cb(device_id, {
                        text,
                        is_over: true,
                        texts
                    });
                    devLog && console.log('LLM connect close!\n');
                    return;
                }
                
                if (line.startsWith('data: ')) {
                    try {
                        const message = JSON.parse(line.replace(/^data: /, ''));
                        const chunk_text = message.choices[0]?.delta?.content || '';

                        if (chunk_text) {
                            devLog && console.log('LLM 输出 ：', chunk_text);
                            texts.count_text += chunk_text;
                            cb(device_id, { text, texts });
                        }
                    } catch (e) {
                        console.error('Error parsing message:', line, e);
                    }
                }
            }
        });

        res.on('end', () => {
            cb(device_id, {
                text,
                is_over: true,
                texts
            });
            devLog && console.log('\n===\n', texts.all_text, '\n===\n');
        });
    });

    req.on('error', (err) => {
        console.log("LLM connect err: " + err);
        if (onError) onError(err);
    });

    // 写入请求体
    req.write(body);
    req.end();
}

module.exports = LLM_FN;
