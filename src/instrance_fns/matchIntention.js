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
    const { devLog, ai_server } = G_config;
    const TTS_FN = require(`../functions/tts`);
    const { ws: ws_client, user_config, llm_historys = [], prev_play_audio_ing, user_config: { intention = [] } } = G_devices.get(device_id);
    if (!text) return null;
    let task_info = null;

    // 推理中
    G_devices.set(device_id, { ...G_devices.get(device_id), intention_ing: true })

    intention_for: for (const item of intention) {
        const { key = [] } = item;
        if (typeof key === "function") {
            const res = await key(text, { llm_historys, prev_play_audio_ing, instance: G_Instance, device_id });
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
                const response = await axios.post(item.nlp_server || `${ai_server}/ai_api/semantic`, {
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

    // 结束推理
    G_devices.set(device_id, { ...G_devices.get(device_id), intention_ing: false })

    if (task_info) {
        const { instruct, data, pin, __name__, music_server, on_end, target_device_id, api_key, channel, deg } = task_info;
        if (typeof instruct === "function") {
            await instruct({
                text,
                instance: G_Instance,
                device_id
            });
        } else {
            switch (instruct) {
                case "__sleep__":
                    if (!G_devices.get(device_id)) return;
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        stop_next_session: true,
                    })
                    break;
                case "__io_high__":
                case "__io_low__":
                    (!pin && pin !== 0) && log.error(`${instruct} 指令必须配置 pin`);
                    if (target_device_id) {
                        !api_key && log.error(`指定了 target_device_id 的指令必须配置 api_key。`);
                        // const response = await axios.get(`http://192.168.3.23:7002/sdk/pin?target_device_id=${target_device_id}&api_key=${api_key}&instruct=${instruct}`, {
                        const response = await axios.get(`${ai_server}/sdk/pin?target_device_id=${target_device_id}&api_key=${api_key}&instruct=${instruct}`, {
                            headers: { 'Content-Type': 'application/json' }
                        });
                        const { success, message: res_msg } = response.data;
                        if (!success) {
                            await TTS_FN(device_id, {
                                text: res_msg,
                                need_record: false,
                                text_is_over: true,
                            });
                        }
                    } else {
                        G_Instance.digitalWrite(device_id, pin, instruct === "__io_high__" ? "HIGH" : "LOW");
                    }
                    break;
                case "__LEDC__":
                    // LEDC 暂不支持远程设备
                    (!channel && channel !== 0) && log.error(`${instruct} 指令必须配置 channel、deg`);
                    G_Instance.ledcWrite(device_id, channel, deg);
                    break;
                case "__play_music__":
                    // 所有 LLM 用下面的 key 为准 
                    if (!G_devices.get(device_id)) return;
                    G_Instance.stop(device_id, "__play_music__");

                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        llm_historys,
                        stop_next_session: true
                    })

                    const { url, seek, message: errMessage } = await music_server(__name__ || text, { user_config, instance: G_Instance, device_id });
                    // 等待说话完毕
                    // test...
                    


                    if (!url) {
                        await TTS_FN(device_id, {
                            text: errMessage || "没有找到相关的结果，换个关键词试试吧！",
                            need_record: true,
                        });
                        return;
                    }
                    try {
                        const session_id = await G_Instance.newSession(device_id);
                        play_audio(url, ws_client, "play_music", session_id, device_id, seek, on_end)
                    } catch (err) {
                        log.error(`音频播放过程失败： ${err}`)
                        await TTS_FN(device_id, {
                            text: "音频播放出错啦，重新换一首吧！",
                            need_record: true,
                        });
                    }

                    break;
                default:
                    devLog && log.iat_info(`执行指令：${instruct}, data: ${data}, name: ${__name__}`);
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        // stop_next_session: true
                    })
                    instruct && ws_client && ws_client.send(JSON.stringify({
                        type: "instruct",
                        command_id: instruct,
                        data: data,
                        name: __name__
                    }));
                    break;
            }
        }
    }

    return task_info;
}
module.exports = matchIntention;
