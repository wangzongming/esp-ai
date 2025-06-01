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
    try {
        !device_id && log.error(`调用 matchIntention 方法时，请传入 device_id`);
        const { devLog, api_key: g_api_key, ai_server } = G_config;
        const { ws: ws_client, user_config, start_iat, llm_historys = [], prev_play_audio_ing, api_key: user_api_key, user_config: { intention = [] } } = G_devices.get(device_id);
        if (!text) return null;
        let task_info = null;

        const api_key = user_api_key || g_api_key;

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
                } else {
                    // AI 推理 
                    const response = await axios.post(item.nlp_server || `${ai_server}/ai_api/semantic`, {
                        "api_key": item.api_key || api_key,
                        "texts": [key[0], _text]
                    }, {
                        headers: { 'Content-Type': 'application/json' },
                    });
                    const { success, message: res_msg, data } = response.data;
                    if (!success) {
                        await G_Instance.tts(device_id, res_msg);
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
            // console.log('task_info', task_info)
            G_devices.set(device_id, { ...G_devices.get(device_id), stop_next_session: true })
            // 等待说话完毕  
            await G_Instance.awaitPlayerDone(device_id);
            todo();
            async function todo() {
                const { instruct, data, pin, __name__, music_server, on_end, target_device_id, api_key, channel, deg } = task_info;
                if (typeof instruct === "function") {
                    await instruct({
                        text,
                        instance: G_Instance,
                        device_id
                    });
                    await G_Instance.awaitPlayerDone(device_id);
                    await start_iat();
                } else {
                    switch (instruct) {
                        case "__sleep__":
                            if (!G_devices.get(device_id)) return;
                            break;
                        case "__io_high__":
                        case "__io_low__":
                            (!pin && pin !== 0) && log.error(`${instruct} 指令必须配置 pin`);
                            if (target_device_id) {
                                !api_key && log.error(`指定了 target_device_id 的指令必须配置 api_key。`);
                                const response = await axios.get(`${ai_server}/sdk/pin?target_device_id=${target_device_id}&api_key=${api_key}&instruct=${instruct}`, {
                                    headers: { 'Content-Type': 'application/json' }
                                });
                                const { success, message: res_msg } = response.data;
                                if (!success) {
                                    await G_Instance.tts(device_id, res_msg);
                                }
                            } else {
                                G_Instance.digitalWrite(device_id, pin, instruct === "__io_high__" ? "HIGH" : "LOW");
                            }
                            await (() => new Promise((rsolve) => setTimeout(rsolve, 500)))()
                            await G_Instance.awaitPlayerDone(device_id);
                            await (() => new Promise((rsolve) => setTimeout(rsolve, 500)))()
                            await start_iat();
                            break;
                        case "__LEDC__":
                            // LEDC 暂不支持远程设备
                            (!channel && channel !== 0) && log.error(`${instruct} 指令必须配置 channel、deg`);
                            G_Instance.ledcWrite(device_id, channel, deg);

                            await G_Instance.awaitPlayerDone(device_id);
                            await (() => new Promise((rsolve) => setTimeout(rsolve, 500)))()
                            await start_iat();
                            break;
                        case "__play_music__":
                            // 所有 LLM 用下面的 key 为准 
                            if (!G_devices.get(device_id)) return;

                            // 创建 AbortController 实例
                            const controller = new AbortController();
                            const signal = controller.signal;

                            const { abort_controllers = [] } = G_devices.get(device_id);
                            G_devices.set(device_id, { ...G_devices.get(device_id), abort_controllers: [...abort_controllers, controller] })

                            const { url, seek, message: errMessage } = await music_server(__name__ || text, {
                                user_config, instance: G_Instance, device_id, signal,
                                sendToClient: (_text) => ws_client && ws_client.send(JSON.stringify({
                                    type: "instruct",
                                    command_id: "on_llm_cb",
                                    data: _text 
                                }))
                            });
                            if (signal && signal.aborted) return;
                            await G_Instance.awaitPlayerDone(device_id);
                            await (() => new Promise((rsolve) => setTimeout(rsolve, 500)))()
                            if (signal && signal.aborted) return;
                            if (!url) {
                                await G_Instance.tts(device_id, errMessage || "没有找到相关的结果，换个关键词试试吧！");
                                if (signal && signal.aborted) return;
                                await G_Instance.awaitPlayerDone(device_id);
                                if (signal && signal.aborted) return;
                                await (() => new Promise((rsolve) => setTimeout(rsolve, 500)))()
                                if (signal && signal.aborted) return;
                                await start_iat();
                                return;
                            }
                            try {
                                const session_id = await G_Instance.newSession(device_id);
                                if (signal && signal.aborted) return;
                                play_audio(url, ws_client, G_session_ids["play_music"], session_id, device_id, seek, async (...args) => {
                                    if (signal && signal.aborted) return;
                                    await (() => new Promise((rsolve) => setTimeout(rsolve, 500)))()
                                    if (signal && signal.aborted) return;

                                    await G_Instance.tts(device_id, "歌曲怎么样？可以和我聊聊你的感受吗？");

                                    if (signal && signal.aborted) return;
                                    await G_Instance.awaitPlayerDone(device_id);
                                    on_end && on_end(...args);
                                    if (signal && signal.aborted) return;
                                    await start_iat();
                                })
                            } catch (err) {
                                log.error(`音频播放过程失败： ${err}`)
                                await G_Instance.tts(device_id, "音频播放出错啦，重新换一首吧！");
                                if (signal && signal.aborted) return;
                                await G_Instance.awaitPlayerDone(device_id);
                                if (signal && signal.aborted) return;
                                await start_iat();
                            }

                            break;
                        default:
                            devLog && log.iat_info(`执行指令：${instruct}, data: ${data}, name: ${__name__}`);
                            instruct && ws_client && ws_client.send(JSON.stringify({
                                type: "instruct",
                                command_id: instruct,
                                data: data,
                                name: __name__
                            }));
                            await G_Instance.awaitPlayerDone(device_id);
                            await start_iat();
                            break;
                    }
                }
            }
        } else {
            G_devices.set(device_id, { ...G_devices.get(device_id), intention_ing: false })
        }

        return task_info;
    } catch (err) {
        console.log(err);
        log.error(`matchIntention.js 错误： ${err}`)
    }

}
module.exports = matchIntention;
