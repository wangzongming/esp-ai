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

const log = require("../../utils/log");

/**
 * 接下来的 session_id 都当前形参为准
*/
async function cb(device_id, { text, user_text, is_over, texts, chunk_text, session_id }) {
    try {
        const { devLog, onLLMcb, llm_qa_number } = G_config;
        if (!G_devices.get(device_id)) return;
        const TTS_FN = require(`../tts`);
        const { llm_historys = [], ws: ws_client, llm_ws, tts_buffer_chunk_queue, tts_server } = G_devices.get(device_id);
        // if(stoped) return;
        if (!texts.index) {
            texts.index = 0;
        }


        onLLMcb && onLLMcb({
            device_id,
            text: chunk_text,
            user_text,
            llm_text: texts.count_text,
            is_over, llm_historys, ws: ws_client,
            instance: G_Instance,
            sendToClient: (_chunk_text) => ws_client && ws_client.send(JSON.stringify({
                type: "instruct",
                command_id: "on_llm_cb",
                data: _chunk_text || chunk_text
            }))
        });


        // 截取TTS算法需要累计动态计算每次应该取多少文字转TTS，而不是固定每次取多少
        const notPlayText = texts.count_text.substr(texts.all_text.length);

        if (is_over) {
            devLog && log.llm_info('-> LLM 推理完毕');
            llm_ws && llm_ws.close()
            // 最后在检查一遍确认都 tts 了，因为最后返回的字数小于播放阈值可能不会被播放，所以这里只要不是空的都需要播放
            const { speak_text: ttsText = "", org_text = "" } = extractBeforeLastPunctuation(notPlayText, true, 0, tts_server)

            const textNowNull = ttsText.replace(/\s/g, '') !== "";
            // console.log('llm 结束: ', textNowNull, ttsText);
            if (textNowNull) {
                // 添加音频播放任务
                tts_buffer_chunk_queue && tts_buffer_chunk_queue.push(async () => {
                    const tts_res = await TTS_FN(device_id, {
                        text: ttsText,
                        pauseInputAudio: true,
                        session_id,
                        text_is_over: true,
                        need_record: true
                    })
                    return tts_res;
                })
                texts.all_text += org_text;
            } else {
                // 特殊结束任务
                tts_buffer_chunk_queue && tts_buffer_chunk_queue.push(() => {
                    devLog && log.tts_info(`-> 服务端发送 LLM 结束的标志流: ${G_session_ids["tts_all_end_align"]}`);
                    ws_client.send(Buffer.from(G_session_ids["tts_all_end_align"], 'utf-8'));
                    return true;
                })
            }

            // 所有 LLM 用下面的 key 为准
            llm_historys.push(
                { "role": "user", "content": text },
                { "role": "assistant", "content": texts.all_text }
            );

            if (llm_historys.length > (llm_qa_number * 2)) { 
                llm_historys.shift();
                llm_historys.shift();
            }

            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                llm_historys,
                llm_ws: null,
                // llm 推理状态
                llm_inference_ing: false
            })
            ws_client && ws_client.send(JSON.stringify({
                type: "session_status",
                status: "llm_end",
            }));
        }
        else {
            const { speak_text: ttsText = "", org_text = "" } = extractBeforeLastPunctuation(notPlayText, false, texts.index, tts_server)
            if (ttsText) {
                // log.llm_info('客户端播放1：', ttsText);
                texts.all_text += org_text;
                texts.index += 1;

                // 添加任务
                tts_buffer_chunk_queue && tts_buffer_chunk_queue.push(() => {
                    return TTS_FN(device_id, {
                        text: ttsText,
                        pauseInputAudio: true,
                        session_id,
                        text_is_over: false,
                        need_record: false
                    })
                })
            }

        }
    } catch (err) {
        console.log(err);
        log.error(`[${device_id}] LLM 回调错误： ${err}`)
    }


}


/**
 * 文本截取函数: 这里用多用一次 TTS 额度来保证速度
 *  1. 使用正则表达式匹配所有表示句子停顿的中英文标点,并不是使用标点符号分割
 *  2. 一些特殊符号不用念出来
*/
function extractBeforeLastPunctuation(str, isLast, index, tts_server) {
    // 匹配句子结束的标点，包括中英文，并考虑英文句号后的空格
    const punctuationRegex = /[\.,;!?)>"‘”》）’!?】。、，；！？》）”’] ?/g;
    const matches = [...str.matchAll(punctuationRegex)];

    if (!isLast && matches.length === 0) return {};
    const notSpeak = /[\*|\n]/g;
    // 获取最后一个匹配的标点符号的索引
    const lastIndex = matches[matches.length - 1]?.index;
    if (lastIndex || lastIndex === 0) {
        const res = str.substring(0, lastIndex + 1);
        // 这里是否考虑提供配置让用户决策
        const min_len = (index === 1 ? 10 : Math.min(index * 30, 300));
        if ((res.length < min_len) && !isLast) {
            return {}
        }
        let speak_text = res.length > 0 ? res.replace(notSpeak, '') : "";
        return { speak_text, org_text: res }
    } else {
        // 如果没有找到标点符号，并且是最后一段文字，则返回整个字符串
        if (!isLast) {
            return {};
        }
        // 表情符号目前还没有好的方式去处理...
        const emojiRegex = /[\u{1F300}-\u{1F6FF}|\u{1F900}-\u{1F9FF}|\u{2600}-\u{26FF}|\u{2700}-\u{27BF}|\u{1F680}-\u{1F6C0}]/ug;

        let speakText = str.replace(notSpeak, '').replace(emojiRegex, '');
        return { speak_text: speakText, org_text: str };
    }


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
        const plugin = plugins.find(item => item.name === llm_server && item.type === "LLM")?.main;
        let LLM_FN = null;

        devLog && log.info("");
        devLog && log.llm_info('-> 开始请求 LLM 输入: ', text);

        LLM_FN = plugin || require(`./${llm_server}`);
        onLLM && onLLM({
            device_id,
            text,
            ws: ws_client,
            instance: G_Instance,
            sendToClient: () => ws_client && ws_client.send(JSON.stringify({
                type: "instruct",
                command_id: "on_llm",
                data: text
            }))
        });

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


        /**
         * 开始连接 llm 服务的回调
        */
        const connectServerBeforeCb = () => {
            if (!G_devices.get(device_id)) return;
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                llm_server_connect_ing: true,
            })
        }


        /**
         * 连接 llm 服务后的回调
        */
        const connectServerCb = (connected) => {
            if (connected) {
                if (!G_devices.get(device_id)) return;
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    llm_server_connected: true,
                    llm_server_connect_ing: false,
                })
            } else {
                if (!G_devices.get(device_id)) return;
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    llm_server_connected: false,
                    llm_server_connect_ing: false,
                })
            }
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
            connectServerBeforeCb,
            connectServerCb,
            cb: (args) => cb(device_id, { ...args, user_text: text, session_id })
        })
    } catch (err) {
        console.log(err);
        log.error(`LLM 错误： ${err}`)
    }

};
