/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */

const max_his = 6 * 2;
const ttsMinNum = 2;
const play_temp = require(`../../audio_temp/play_temp`);
const log = require("../../utils/log");

async function cb(device_id, { text, is_over, texts }) {
    const { devLog, llm_server, onLLMcb } = G_config;
    const TTS_FN = require(`../tts`);

    const { llm_historys = [], ws: ws_client, start_iat, llm_ws, await_out_tts, await_out_tts_run, add_audio_out_over_queue } = G_devices.get(device_id);

    onLLMcb && onLLMcb({ device_id, text, is_over, llm_historys });

    /**
      * 实时播放逻辑：
      * 这里需要用标点符号来做分割符号，一句一句翻译为 tts。 这样才能保证速度 
      * 播放过的不播放，为了保证性能需要控制最少播放字数阈值
     */
    const notPlayText = texts.count_text.substr(texts.all_text.length)
    // 截取最后一个标点符号前面的所有字符
    const ttsText = extractBeforeLastPunctuation(notPlayText)
    devLog && ttsText && log.llm_info('客户端播放：', ttsText);
    texts.all_text += ttsText;


    ttsText && await_out_tts.push(async () => {
        await TTS_FN(device_id, {
            text: ttsText,
            reRecord: false,
            pauseInputAudio: true
        })
    })
    ttsText && await_out_tts_run();
    ttsText && G_devices.set(device_id, {
        ...G_devices.get(device_id),
        await_out_tts_ing: true
    })


    if (is_over) {
        devLog && log.llm_info('-> LLM 推理完毕');
        llm_ws && llm_ws.close()
        // 最后在检查一遍确认都 tts 了，因为最后返回的字数小于播放阈值可能不会被播放，所以这里只要不是空的都需要播放
        const ttsText = extractBeforeLastPunctuation(notPlayText, true)
        if ((ttsText.length < ttsMinNum) && ttsText) {
            await_out_tts.push(async () => {
                await TTS_FN(device_id, {
                    text: ttsText,
                    reRecord: false,
                    pauseInputAudio: true
                })
            })
            ttsText && await_out_tts_run();
            ttsText && G_devices.set(device_id, {
                ...G_devices.get(device_id),
                await_out_tts_ing: true
            })
        }

        await_out_tts.push(async () => { 
            add_audio_out_over_queue("warning_tone", () => { 
                start_iat && start_iat();
            })
            await play_temp("du.pcm", ws_client); 
        })
        ttsText && await_out_tts_run();

        switch (llm_server) {
            case "xun_fei":
            case "dashscope":
                llm_historys.push(
                    { "role": "user", "content": text },
                    { "role": "assistant", "content": texts.all_text }
                );
                break;
            default:
                break;
        }

        llm_historys.length > max_his && llm_historys.shift();

        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            llm_historys,
            llm_ws: null
        })
    }
}

 
/**
 * 文本截取函数
 * 使用正则表达式匹配所有表示句子停顿的中英文标点
 * 一些特殊符号不用念出来
*/
function extractBeforeLastPunctuation(str, isLast) {
    const matches = [...str.matchAll(/[\.,;!?()<>"‘。、，；！？《》（）“”‘’]/g)];

    // 不是最后一个流，并且没有达到字数阈值
    if (!isLast && str.length <= ttsMinNum) return "";
    // 不是最后一个流，并且检测不出来符号
    if (!isLast && matches.length === 0) return "";

    const notSpeek = /[\*|\\n]/g;

    // 获取最后一个匹配的标点符号的索引
    const lastIndex = matches[matches.length - 1]?.index;
    if (lastIndex == null) return "";

    // 返回最后一个标点符号前的所有字符， 还需要包括标点符号。
    // 返回时包括了符号，所以只有一个符号的情况下是不能返回的
    // 避免有卡顿感，少于 x 个字也不做返回
    const res = str.substring(0, lastIndex + 1);
    if (isLast) {
        // 最后一个流时，只要不是空就得播放, 
        return res.length > 0 ? res.replace(notSpeek, '') : ""
    } else {
        return res.length >= ttsMinNum ? res.replace(notSpeek, '') : ""
    }
}
module.exports = (device_id, opts) => {
    const TTS_FN = require(`../tts`);
    const { devLog, llm_server, api_key, plugins = [], llm_init_messages = [], llm_params_set } = G_config;
    const { llm_historys = [], ws: ws_client } = G_devices.get(device_id);
    const { text } = opts;
    const plugin = plugins.find(item => item.name == llm_server && item.type === "LLM")?.main;
    let LLM_FN = null;


    devLog && log.llm_info("");
    devLog && log.llm_info('=== 开始请求 LLM 输入: ', text, " ===");
    if (!api_key[llm_server]) {
        log.error(`请先配置好 ${llm_server} 服务的 api_key`);
        return;
    }
    LLM_FN = plugin || require(`./${llm_server}`);

    /**
     * llm 服务发生错误时调用
    */
    const llmServerErrorCb = (err) => {
        log.error(err)
        TTS_FN(device_id, {
            text: "大语言模型服务发生错误",
            reRecord: false,
            pauseInputAudio: true,
            onAudioOutOver: () => {
                ws_client && ws_client.send("session_end");
                ws_client && ws_client.send("iat_end");
            }
        })

    }
    /**
     * 记录 llm 服务对象
     */
    const logWSServer = (wsServer) => {
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            llm_ws: wsServer,
        })
    }
 
    return LLM_FN({
        device_id,
        devLog,
        log,
        text,
        llm_historys,
        llm_init_messages,
        llm_params_set,
        api_key: api_key[llm_server],
        logWSServer,
        llmServerErrorCb,
        cb: (args) => cb(device_id, { ...args })
    })
};
