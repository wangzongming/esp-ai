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

const https = require('https');
const http = require('http');
const log = require('../utils/log');
const { PassThrough } = require('stream');
const moment = require('moment');

function isHttpUrl(url) {
    const regex = /^https?:\/\/.+/;
    return regex.test(url);
}

function isHttpsUrl(url) {
    const regex = /^https:\/\/.+/;
    return regex.test(url);
}

function urlToStream(url, errCb, onEnd) {
    try {
        const stream = new PassThrough();
        const hf = isHttpsUrl(url) ? https : http;
        hf.get(url, {
            timeout: 100000,
        }, (response) => {
            // console.log(response)
            if (response.statusCode !== 200) {
                log.error(`音频地址不可用：${url}`)
                stream.emit('error', new Error(`Request failed with status code ${response.statusCode}`));
                return;
            }
            response.pipe(stream);
        }).on('error', (err) => {
            errCb && errCb();
            log.error(`音频播放错误：${err}`)
            stream.emit('error', err);
        }).on("close", () => {
            log.t_info('音频下载完毕');
            onEnd && onEnd();
        });

        return stream;
    } catch (err) {
        console.log(err)
        log.error(`音频播放错误: ${err}`)
        errCb && errCb();
    }
}
/**
 * 播放任何 mp3、wav 的声音
 * 提供 http 地址
*/
function play_audio(url, client_ws, task_id, session_id, device_id, seek, on_end) {
    let checkBufferedAmount = null;
    try {
        if (!isHttpUrl(url)) {
            log.error("play_audio 不支持本地地址！")
            return;
        }

        client_ws && client_ws.send(JSON.stringify({
            type: "session_status",
            status: "tts_chunk_start",
        }));
        if (!G_devices.get(device_id)) return;

        // 结束时一定要调用本函数
        let ended = false;
        async function endCb(event = "play_end") {
            if (!G_devices.get(device_id)) return;
            if (ended) return;
            ended = true;
            const { start_audio_time } = G_devices.get(device_id);
            // 真正结束
            const end_time = Date.now(); // 结束时间
            const play_time = end_time - start_audio_time; // 播放时间
            const end_res = await on_end({
                start_time: start_audio_time,
                end_time: end_time,
                play_time: Math.floor(play_time / 1000),
                break_second: Math.floor(seek + play_time / 1000),
                event,
                seek
            }) 
            
            // 特殊发一组标志 
            this.ws.send(Buffer.from(G_session_ids.tts_all_end, 'utf-8')); 

            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                first_session: true,
                play_audio_ing: false,
                play_audio_on_end: null,
            })

            if (end_res?.url) {
                play_audio(end_res?.url, client_ws, task_id, session_id, device_id, end_res?.seek, on_end);
            }

        }

        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            play_audio_ing: true,
            play_audio_on_end: (...arg) => { 
                this.ws.send(Buffer.from(G_session_ids.tts_all_end, 'utf-8'));  
                return on_end && on_end(...arg);
            },
            play_audio_seek: seek || 0,
        })

        client_ws.send(JSON.stringify({ type: "play_audio", tts_task_id: task_id || "any_audio" }));

        let is_parse_over = false;
        const output_stream = urlToStream(url,
            () => endCb("error"),
            () => {
                is_parse_over = true;
            }
        );

        let buffer_queue = [];
        let is_sending = false;
        let is_first_on_data = true;
        const session_id_buffer = Buffer.from(session_id, 'utf-8');
        output_stream.on('data', function (audio) {
            if (is_first_on_data) {
                if (!G_devices.get(device_id)) return;
                is_first_on_data = false;
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    start_audio_time: Date.now(),
                })
            }
            let c_l = G_max_audio_chunk_size;
            for (let i = 0; i < audio.length; i += c_l) {
                if (!G_devices.get(device_id)) return;

                const { session_id: now_session_id } = G_devices.get(device_id);
                if ((session_id && now_session_id !== session_id)) {
                    log.t_info("用户终止流")
                    buffer_queue = [];
                    output_stream.end();
                    break;
                }

                const end = Math.min(i + c_l, audio.length);
                const chunk = audio.slice(i, end);
                if (!(Buffer.isBuffer(chunk))) {
                    log.t_info(`跳过无效 chunk: ${i}`);
                    continue;
                }
                buffer_queue.push(chunk);
                !is_sending && sendNextChunk();
            }
        });

        // 处理发送缓冲区是否清空
        function sendNextChunk() {
            if (buffer_queue.length > 0) {
                if (!G_devices.get(device_id)) return;
                const { session_id: now_session_id } = G_devices.get(device_id);
                if ((session_id && now_session_id !== session_id)) {
                    log.t_info("用户终止对话，清空音频缓冲区")
                    buffer_queue = [];
                    return;
                }

                is_sending = true;
                const send_chunk = buffer_queue.shift();
                const real_chunk = Buffer.concat([session_id_buffer, send_chunk])
                // console.log("发送音频：", moment().format("HH:mm:ss:sss"),  real_chunk.length);
                client_ws.send(real_chunk, (err) => {
                    if (is_parse_over) {
                        if (!G_devices.get(device_id)) return;
                        if (buffer_queue.length === 0) {
                            endCb();
                        }
                    }

                    if (err) {
                        console.error('发送数据出错:', err);
                    }
                    // 等待 WebSocket 缓冲区清空后继续发送 
                    clearInterval(checkBufferedAmount);
                    checkBufferedAmount = setInterval(() => {
                        if (client_ws.bufferedAmount === 0) {
                            clearInterval(checkBufferedAmount);
                            is_sending = false;
                            sendNextChunk();
                        }
                    }, 10);
                });
            }
        }

    } catch (err) {
        clearInterval(checkBufferedAmount);
        if (!G_devices.get(device_id)) return;
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            play_audio_ing: false,
            play_audio_on_end: null,
        })
        log.error("音频播放失败：" + err);
    }
}
module.exports = play_audio;