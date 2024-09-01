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

const max_his = 20 * 1;
const play_temp = require(`../../audio_temp/play_temp`);
const log = require("../../utils/log");
const delay = require("../../utils/delay");

/**
 * 接下来的 session_id 都当前形参为准
*/
async function cb(device_id, { text, is_over, texts, session_id, shouldClose }) {
    try {
        const { devLog, onLLMcb } = G_config;
        const TTS_FN = require(`../tts`);
        const {
            llm_historys = [], ws: ws_client, start_iat, llm_ws, await_out_tts, await_out_tts_run, add_audio_out_over_queue,
            // user_config: { llm_server }
        } = G_devices.get(device_id);
        if (!texts.index) {
            texts.index = 0;
        }
        onLLMcb && onLLMcb({ device_id, text, is_over, llm_historys, ws: ws_client });

        // 截取TTS算法需要累计动态计算每次应该取多少文字转TTS，而不是固定每次取多少 
        const notPlayText = texts.count_text.substr(texts.all_text.length) 
        if (is_over) {
            devLog && log.llm_info('-> LLM 推理完毕');
            llm_ws && llm_ws.close()
            // 最后在检查一遍确认都 tts 了，因为最后返回的字数小于播放阈值可能不会被播放，所以这里只要不是空的都需要播放
            const { speak_text: ttsText = "", org_text = "" } = extractBeforeLastPunctuation(notPlayText, true, 0) 
            
            if (ttsText) {
                await_out_tts.push(async () => {
                    if(shouldClose) return;
                    await TTS_FN(device_id, {
                        text: ttsText,
                        reRecord: false,
                        pauseInputAudio: true,
                        session_id,
                        text_is_over: true,
                        need_record: true
                    })
                })
                texts.all_text += org_text;
                await_out_tts_run();
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    await_out_tts_ing: true
                })
            }else{  
                devLog && log.tts_info(`-> 服务端发送LLM结束的标志流: 2000`);
                const endFlagBuf = Buffer.from("2000", 'utf-8');
                ws_client.send(endFlagBuf);
            }
 

            // 所有 LLM 用下面的 key 为准
            llm_historys.push(
                { "role": "user", "content": text },
                { "role": "assistant", "content": texts.all_text }
            );

            llm_historys.length > max_his && llm_historys.shift();

            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                llm_historys,
                llm_ws: null
            })
        }
        else {
            const { speak_text: ttsText = "", org_text = "" } = extractBeforeLastPunctuation(notPlayText, false, texts.index)
            if (ttsText) {
                // log.llm_info('客户端播放：', ttsText); 
                texts.all_text += org_text;
                texts.index += 1;
                await_out_tts.push(async () => {
                    await TTS_FN(device_id, {
                        text: ttsText,
                        reRecord: false,
                        pauseInputAudio: true,
                        session_id,
                        text_is_over: false,
                        need_record: true
                    })
                })
                await_out_tts_run();
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    await_out_tts_ing: true
                })
            } 

        }
    } catch (err) {
        console.log(err);
        log.error(`LLM 回调错误： ${err}`)
    }


}


/**
 * 文本截取函数: 这里用多用一次 TTS 额度来保证速度
 *  1. 使用正则表达式匹配所有表示句子停顿的中英文标点,并不是使用标点符号分割
 *  2. 一些特殊符号不用念出来
*/
function extractBeforeLastPunctuation(str, isLast, index) {
    const matches = [...str.matchAll(/[\.,;!?)>"‘。、，；！？》）”’]/g)];
    if (!isLast && matches.length === 0) return {};
    const notSpeek = /[\*|\\n]/g;

    // 获取最后一个匹配的标点符号的索引
    const lastIndex = matches[matches.length - 1]?.index;
    if (lastIndex == null) return {};

    const res = str.substring(0, lastIndex + 1);
    const min_len = index === 1 ? 10 : Math.min(index * 50, 500);
    if ((res.length < min_len) && !isLast) {
        return {}
    }
    let speak_text = res.length > 0 ? res.replace(notSpeek, '') : "";
    return { speak_text, org_text: res }
}
module.exports = (device_id, opts) => {
    try {
        const TTS_FN = require(`../tts`);
        const { devLog, plugins = [], llm_params_set, onLLM } = G_config;
        const {
            llm_historys = [],
            ws: ws_client, session_id, error_catch,
            user_config: { iat_server, llm_server, tts_server, llm_config, llm_init_messages = [] }
        } = G_devices.get(device_id);

        const { text } = opts;
        const plugin = plugins.find(item => item.name == llm_server && item.type === "LLM")?.main;
        let LLM_FN = null;

        devLog && log.info("");
        devLog && log.llm_info('=== 开始请求 LLM 输入: ', text, " ===");

        LLM_FN = plugin || require(`./${llm_server}`);
        onLLM && onLLM({ device_id, text, ws: ws_client });

        /**
         * llm 服务发生错误时调用
        */
        const llmServerErrorCb = (err) => {
            log.error(err)
            error_catch("LLM", "202", err);

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

        if (devLog) {
            llm_historys.length && log.llm_info(`----------------------对话记录---------------------------`)
            for (let i = 0; i < llm_historys.length; i++) {
                const item = llm_historys[i];
                log.llm_info(`${item.role}: ${item.content?.replace?.(/[\n]/g, '')}`)
            }
            llm_historys.length && log.llm_info(`--------------------------------------------------------`)
        }

        return LLM_FN({
            device_id,
            devLog,
            log,
            text,
            llm_historys,
            llm_init_messages,
            llm_params_set,
            llm_config,
            iat_server, llm_server, tts_server,
            logWSServer,
            llmServerErrorCb,
            cb: (args) => cb(device_id, { ...args, session_id })
        })
    } catch (err) {
        console.log(err);
        log.error(`LLM 错误： ${err}`)
    }

};
