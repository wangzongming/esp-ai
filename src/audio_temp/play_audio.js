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
const Audio_sender = require("../utils/audio_sender");

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
 * 播放任何 mp3、wav 的声音 (<=128波特率)
 * 提供 http 地址
*/

function play_audio(url, client_ws, task_id, session_id, device_id, seek, on_end) {
    let send_end_buffer_timer = null;
    try {
        if (!isHttpUrl(url)) {
            log.error("play_audio 不支持本地地址！")
            return;
        }
        if (!G_devices.get(device_id)) return;

        let start_audio_time = null;
        const audio_sender = new Audio_sender(client_ws, device_id);
        audio_sender.startSend(session_id);
        client_ws && client_ws.send(JSON.stringify({ type: "play_audio", tts_task_id: task_id }));
        client_ws && client_ws.send(JSON.stringify({ type: "session_status", status: "tts_chunk_start" }));

        // 结束时一定要调用本函数
        let ended = false;
        async function endCb(event = "play_end") {
            if (!G_devices.get(device_id)) return;
            if (ended) return;
            ended = true;
            // 真正结束
            const end_time = Date.now(); // 结束时间
            const play_time = end_time - start_audio_time; // 播放时间
            const end_res = on_end && await on_end({
                start_time: start_audio_time,
                end_time: end_time,
                play_time: Math.floor(play_time / 1000),
                break_second: Math.floor(seek + play_time / 1000),
                event,
                seek
            })

            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                first_session: true,
                play_audio_on_end: null,
            })

            if (end_res?.url) {
                play_audio(end_res?.url, client_ws, task_id, session_id, device_id, end_res?.seek, on_end);
            }
        }

        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            // 播放音频的参数
            play_audio_on_end: (event) => {
                audio_sender.sendAudio(null, G_session_ids["tts_all_end"]);

                audio_sender.stop();
                endCb(event);
            },
            play_audio_seek: seek || 0,
        })

        let is_parse_over = false;
        const output_stream = urlToStream(url,
            () => {
                endCb("error");
                audio_sender.sendAudio(null, G_session_ids["tts_all_end"]);
                audio_sender.stop();
            },
            () => {
                is_parse_over = true;
                audio_sender.sendAudio(null, G_session_ids["tts_all_end"]);
            }
        );

        let is_first_on_data = true;
        output_stream.on('data', function (audio) {
            if (is_first_on_data) {
                if (!G_devices.get(device_id)) return;
                is_first_on_data = false;
                start_audio_time = Date.now();
            }
            audio.length && audio_sender.sendAudio(audio);
        });
    } catch (err) {
        clearTimeout(send_end_buffer_timer);
        if (!G_devices.get(device_id)) return;
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            play_audio_on_end: null,
        })
        log.error("音频播放失败：" + err);
        console.error(err)
    }
}
module.exports = play_audio;
