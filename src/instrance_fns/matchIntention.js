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

const log = require("../utils/log");
const play_audio = require("../audio_temp/play_audio");
const axios = require('axios');

/**
 * 匹配某个命令，如果匹配上会执行
 * 
 *@param reply 回复语，如果是用户手动按按钮的情况下，一般不使用 message，而是使用自定义的提示语
*/
async function matchIntention(device_id, text, reply) {
    !device_id && log.error(`调用 matchIntention 方法时，请传入 device_id`);
    const { devLog, onSleep } = G_config;
    const TTS_FN = require(`../functions/tts`);
    const { ws: ws_client, client_params, session_id, user_config, llm_historys = [], prev_play_audio_ing, user_config: { sleep_reply, intention = [] } } = G_devices.get(device_id);
    if (!text) return null;
    let task_info = null;
    intention_for: for (const item of intention) {
        const { key = [] } = item;
        if (typeof key === "function") {
            const res = await key(text, { llm_historys, prev_play_audio_ing });
            if (res) {
                task_info = item;
                task_info["__name__"] = res;
                break intention_for;
            }
        } else {
            const emojiRegex = /[\u{1F300}-\u{1F6FF}|\u{1F900}-\u{1F9FF}|\u{2600}-\u{26FF}|\u{2700}-\u{27BF}|\u{1F680}-\u{1F6C0}]/ug;
            const punctuationRegex = /[\.,;!?)>"‘”》）’!?】。、，；！？》）”’] ?/g;
            const _text = text.replace(punctuationRegex, '').replace(emojiRegex, '');
            if (key.includes(_text)) {
                // 完全匹配
                task_info = item;
                break intention_for;
            } else if (item.api_key) {
                // AI 推理 
                const response = await axios.post(item.nlp_server || `https://espai.natapp4.cc/v1/semantic`, {
                    "api_key": item.api_key,
                    "texts": [key[0], _text]
                }, {
                    headers: { 'Content-Type': 'application/json' },
                }); 
                const { success, message: res_msg, data } = response.data;
                if (!success) {
                    await TTS_FN(device_id, {
                        text: res_msg,
                        need_record: false,
                        text_is_over: true,
                    });
                } else if (data === true) {
                    task_info = item;
                    break intention_for;
                }
            }
        }
    }
    if (task_info) {
        const { instruct, message, data, pin, __name__, music_server, on_end, target_device_id, api_key } = task_info;
        if (typeof instruct === "function") {
            const resText = await instruct({
                text,
                instance: G_Instance,
                device_id
            });
            let _t = (typeof resText) === "string" ? resText : message;
            _t && await TTS_FN(device_id, {
                text: _t,
                need_record: false,
            });
        } else {
            switch (instruct) {
                case "__sleep__":
                    if (!G_devices.get(device_id)) return;
                    // 所有 LLM 用下面的 key 为准
                    llm_historys.push(
                        { "role": "user", "content": text },
                        { "role": "assistant", "content": message || sleep_reply || "我先退下了，有需要再叫我。" }
                    );
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        first_session: true,
                    })
                    ws_client && ws_client.send("session_end");
                    onSleep && onSleep({ instance: G_Instance, device_id, client_params });
                    devLog && console.log('\n\n === 会话结束 ===\n\n')
                    break;
                case "__io_high__":
                case "__io_low__":
                    !pin && log.error(`__io_high__、__io_low__ 指令必须配置 pin 数据`);
                    // 所有 LLM 用下面的 key 为准
                    llm_historys.push(
                        { "role": "user", "content": text },
                        { "role": "assistant", "content": message || "好的" }
                    );
                    if (target_device_id) {
                        !api_key && log.error(`指定了 target_device_id 的指令必须配置 api_key。`);
                        // const response = await axios.get(`http://192.168.3.23:7002/sdk/pin?target_device_id=${target_device_id}&api_key=${api_key}&instruct=${instruct}`, {
                        const response = await axios.get(`https://api.espai.fun/sdk/pin?target_device_id=${target_device_id}&api_key=${api_key}&instruct=${instruct}`, {
                            headers: { 'Content-Type': 'application/json' }
                        });
                        const { success, message: res_msg } = response.data;
                        if (!success) {
                            await TTS_FN(device_id, {
                                text: res_msg,
                                need_record: false,
                                text_is_over: true,
                            });
                        } else {
                            message && await TTS_FN(device_id, {
                                text: message,
                                need_record: false,
                                text_is_over: true,
                            });
                        }
                    } else {
                        G_Instance.digitalWrite(device_id, pin, instruct === "__io_high__" ? "HIGH" : "LOW");
                        message && await TTS_FN(device_id, {
                            text: message,
                            need_record: false,
                            text_is_over: true,
                        });
                    }
                    break;
                case "__play_music__":
                    // 所有 LLM 用下面的 key 为准
                    llm_historys.push(
                        { "role": "user", "content": text },
                        { "role": "assistant", "content": "好的" }
                    );
                    if (!G_devices.get(device_id)) return;
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        llm_historys,
                    })

                    await TTS_FN(device_id, {
                        text: reply || message || "好的",
                        need_record: false,
                        text_is_over: true,
                    });
                    // 不延时不能保证播放队列
                    setTimeout(async () => {
                        const { url, seek, message: errMessage } = await music_server(__name__, { user_config });
                        if (!url) {
                            await TTS_FN(device_id, {
                                text: errMessage || "没有找到相关的结果，换个关键词试试吧！",
                                need_record: true,
                            });
                            return;
                        }
                        try {
                            play_audio(url, ws_client, "play_music", session_id, device_id, seek, on_end)
                        } catch (err) {
                            log.error(`音频播放过程失败： ${err}`)
                            await TTS_FN(device_id, {
                                text: "音频播放出错啦，重新换一首吧！",
                                need_record: true,
                            });
                        }
                    }, 900);
                    break;
                default:
                    devLog && log.iat_info(`执行指令：${instruct}, data: ${data}, name: ${__name__}`);
                    instruct && ws_client && ws_client.send(JSON.stringify({
                        type: "instruct",
                        command_id: instruct,
                        data: data,
                        name: __name__
                    }));
                    (message || reply) && await TTS_FN(device_id, {
                        text: reply || message,
                        need_record: true,
                        text_is_over: true,
                    });
                    break;
            }
        }
    }
    return task_info;
}
module.exports = matchIntention;