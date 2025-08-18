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

const log = require("./log.js");
const ffmpeg = require('fluent-ffmpeg');
const { PassThrough } = require('stream');
const AudioSaver = require('./audio_saver.js');


/**
 * 音频流发送器
 * 发送速率为 3kb~20kb/100ms~200ms = 10kb/1s
*/
class Audio_sender {
    constructor(ws, device_id) {
        const { client_params = {}, client_version_arr = [] } = G_devices.get(device_id);
        const AUDIO_BUFFER_SIZE = client_params.AUDIO_BUFFER_SIZE || 1024 * 20;

        this.ws = ws;
        this.send_timer = null;
        this.accumulated_data = Buffer.alloc(0);
        this.accumulated_data_mutex = false;
        this.check_count = 0;
        this.started = false;
        this.stoped = false;
        this.stop_rec_audioed = false; // 已经停止接收音频
        this.device_id = device_id;
        this.client_version_arr = client_version_arr;
        // 发送速率，单位 ms
        this.send_speed = 200;
        this.client_max_available_audio = AUDIO_BUFFER_SIZE;

        this.sample_rate = client_params.spk_sample_rate || 16000;
        this.channels = client_params.spk_channels || 1;
        this.format = client_params.spk_format || 'mp3';
        this.bitrate = client_params.spk_bitrate || 128;

        // 音频保存配置(默认关闭)
        this.audioSaver = new AudioSaver(device_id);
        this.audioSaver.toggle(false); // 设置为true，开启保存音频文件（仅当保存功能启用时）

        // 静音处理配置（头部60ms，尾部180ms，平衡间隔与流畅度）
        this.silenceConfig = {
            enabled: true,
            headSilenceDuration: 0.06,   // 头部静音检测时长缩短至60ms
            headSilenceThreshold: '-38dB', // 头部阈值放宽（从-40dB调整为-38dB），减少误判
            tailSilenceDuration: 0.18, // 尾部保持180ms，确保收尾干净
            tailSilenceThreshold: '-37dB', // 尾部阈值放宽（从-40dB调整为-37dB），减少误判
            bufferSize: 1024 * 5,       // 缓冲区大小不变
            flushInterval: 60           // 缩短刷新间隔（从80ms调整为60ms），提升响应速度
        };

        this.not_need_process_data = Number(this.client_version_arr[0]) <= 2 && Number(this.client_version_arr[1]) <= 86;

        if (!this.not_need_process_data) {
            this.end_ss = "";
            this.inputStream = new PassThrough();
            this.outputStream = new PassThrough();

            // 调整后的滤镜链：适配头部60ms静音处理
            const audioFilters = [
                // 头部：1个连续0.06s的静音（≤-38dB）则移除；尾部：1个连续0.18s的静音（≤-37dB）则移除
                `silenceremove=1:${this.silenceConfig.headSilenceDuration}:${this.silenceConfig.headSilenceThreshold}:1:${this.silenceConfig.tailSilenceDuration}:${this.silenceConfig.tailSilenceThreshold}`,
                // 'apad=pad_dur=0.02',  // 缩短填充静音（从50ms调整为20ms），减少句间间隔
            ];

            const ffmpegProcess = ffmpeg(this.inputStream)
                .inputFormat('mp3')
                .audioChannels(this.channels)
                .audioFrequency(this.sample_rate)
                .audioFilters(audioFilters)  // 应用静音处理
                .audioCodec(this.getCodecForFormat(this.format))    // 编码为 MP3（可改为 libopus、adpcm_ima_wav 等）
                .audioBitrate(this.bitrate)            // 比特率
                .format(this.format)
                // .addOption('-q:a', '9')               // 最低质量（输出更快）
                // .addOption('-fflags', 'nobuffer')     // 减少内部缓冲
                // .addOption('-compression_level', '0')
                // .on('error', err => {  console.error('FFmpeg error:', err.message);  })
                // 关键：监听错误事件（避免错误导致进程挂起）
                .on('error', (err) => {
                    log.error('FFmpeg 错误:' + err.message);
                    // 发生错误时主动终止子进程，防止僵尸进程
                    if (ffmpegProcess && !ffmpegProcess.killed && this.ffmpegProcess.kill) {
                        ffmpegProcess.kill('SIGKILL');
                    }
                })
                .on('end', () => {
                    this.accumulated_data = Buffer.concat([this.accumulated_data, Buffer.from(this.end_ss || '', 'utf-8')]);
                })
                .pipe(this.outputStream);
            this.outputStream.on('data', chunk => {
                this.accumulated_data = Buffer.concat([this.accumulated_data, chunk]);

                // 写入处理后音频到保存流（仅当保存功能启用时）
                if (this.audioSaver.isEnabled()) {
                    this.audioSaver.writeProcessed(chunk);
                }
            });

            // 额外：监听输出流的错误，避免流异常导致ffmpeg进程残留
            this.outputStream.on('error', (err) => {
                console.error('输出流错误:', err.message);
                if (ffmpegProcess && !ffmpegProcess.killed && this.ffmpegProcess.kill) {
                    ffmpegProcess.kill('SIGKILL'); // 终止ffmpeg子进程
                }
            });
            this.ffmpegProcess = ffmpegProcess;
        }

    }

    /**
     * 启动音频发送
    */
    startSend(session_id, on_end) {
        return new Promise((resolve) => {
            this.started = true;
            this.check_count = 0;
            this.accumulated_data = Buffer.alloc(0);
            if (!G_devices.get(this.device_id)) return;
            G_devices.set(this.device_id, { ...G_devices.get(this.device_id), play_audio_ing: true, });

            this.sender = () => {
                if (!this.accumulated_data.length) {
                    if (this.check_count >= 100) {
                        clearTimeout(this.send_timer);
                        this.started = false;
                        this.send_timer = null;
                        return;
                    }

                    this.check_count++;
                    this.send_timer = null;
                    setTimeout(this.sender, 50);
                    return;
                };

                if (!G_devices.get(this.device_id)) return;
                if (this.stoped) return;
                const { client_available_audio = 0 } = G_devices.get(this.device_id);

                let send_num = this.client_max_available_audio;
                let send_speed = this.send_speed;

                // 超出最大缓冲区，暂停发送任务
                if (+client_available_audio >= +this.client_max_available_audio) {
                    clearTimeout(this.send_timer);
                    this.send_timer = setTimeout(this.sender, 100);
                    return;
                };
                if (client_available_audio < this.client_max_available_audio / 2 / 2) {
                    send_speed = this.send_speed / 2;
                }

                let max_send = this.client_max_available_audio - client_available_audio;
                if (this.client_max_available_audio >= (20 * 1024)) {
                    // 这里包如果过大会导致设备处理不出来
                    send_num = Math.min(max_send, this.client_max_available_audio / 2);
                } else {
                    // 缓存小，就要传输快一些。
                    send_num = Math.min(max_send, this.client_max_available_audio * 0.8);
                }

                let data = null;
                const remain = this.accumulated_data.length - send_num;
                if (remain > 0 && remain < 4) {
                    data = this.accumulated_data;
                } else {
                    data = this.accumulated_data.slice(0, send_num);
                }

                const { session_id: now_session_id } = G_devices.get(this.device_id);
                if (now_session_id && session_id && now_session_id !== session_id && !([
                    G_session_ids["cache_sleep_reply"],
                    G_session_ids["cache_du"],
                    G_session_ids["cache_hello"],
                    G_session_ids["tts_fn"],
                ].includes(session_id))) return;

                // session status
                let ss = G_session_ids["tts_session"];
                let real_data = data;
                const data_len = data.length;
                const end_data = data.slice(-2);
                const end_data_str = end_data.toString();
                const is_real_end = [G_session_ids["tts_all_end_align"], G_session_ids["tts_all_end"]];
                const is_end = [...is_real_end, G_session_ids["tts_chunk_end"]].includes(end_data_str);
                if (is_end && this.stop_rec_audioed) {
                    ss = end_data_str;
                    // 删除最后两个字节
                    real_data = data.slice(0, data_len - 2);
                }

                // test...
                // log.tts_info("::: 发送-> 会话ID: ", session_id, " 会话状态:", ss, " 数据长度:", data_len, " 客户端有效音频:", Math.floor(client_available_audio / 1024) + "kb", "  send_speed：", send_speed, `最后字节：${real_data[data_len - 1] ||''}`);
                if (!session_id) return log.error(`缺失会话ID，发送失败。`);

                const combinedBuffer = Buffer.concat([Buffer.from(session_id, 'utf-8'), Buffer.from(ss, 'utf-8'), real_data]);
                this.ws.send(combinedBuffer, () => {
                    if (is_end) {
                        clearTimeout(this.send_timer);
                        on_end && on_end();
                        resolve();
                    } else {
                        clearTimeout(this.send_timer);
                        this.send_timer = setTimeout(this.sender, send_speed);
                        this.accumulated_data = this.accumulated_data.slice(data_len);
                    }
                });
            };
            this.sender();
        })
    }


    /**
     * 发送音频函数
     * @param {*} stream_chunk
     * @param {*} status
     */
    sendAudio(stream_chunk, status = "") {
        const is_end = [G_session_ids["tts_all_end_align"], G_session_ids["tts_all_end"], G_session_ids["tts_chunk_end"]].includes(status);

        // 写入原始音频数据到保存流（仅当保存功能启用时）
        if (this.audioSaver.isEnabled()) {
            if (!is_end && stream_chunk?.length > 0) {
                this.audioSaver.writeRaw(stream_chunk); // 写入原始数据
            } else if (is_end) {
                // 音频结束时，标记原始流和处理流完成
                this.audioSaver.finishRaw();
                this.audioSaver.finishProcessed();
            }
        }

        if (is_end) {
            this.stop_rec_audioed = true;
            if (this.not_need_process_data) {
                this.accumulated_data = Buffer.concat([this.accumulated_data, Buffer.from(status, 'utf-8')]);
            } else {
                this.end_ss = status;
                this.inputStream.end();
            }
        } else {
            if (stream_chunk?.length === 0) return;
            // 旧版兼容
            if (this.not_need_process_data) {
                this.accumulated_data = Buffer.concat([this.accumulated_data, stream_chunk]);
            } else {
                this.inputStream.write(stream_chunk);
            }
        }

    }


    getCodecForFormat(format) {
        switch ((format || '').toLowerCase()) {
            // 无压缩 PCM 编码（适用于 ESP32 等嵌入式设备）
            case 'pcm':
            case 'pcm_s16le':  // 16-bit little-endian PCM
            case 'raw':
                return 'pcm_s16le';

            // ADPCM（适用于低带宽语音传输）
            case 'adpcm':
            case 'adpcm_ima_wav':
                return 'adpcm_ima_wav';

            // MP3
            case 'mp3':
                return 'libmp3lame';

            // AAC（部分 FFmpeg 编译需启用 libfdk_aac）
            case 'aac':
                return 'aac';

            // Opus（适用于高质量语音通信）
            case 'opus':
                return 'libopus';

            // Speex（低带宽语音）
            case 'speex':
            case 'ogg_speex':
                return 'libspeex';

            // FLAC（无损音频）
            case 'flac':
                return 'flac';

            // μ-law 编码
            case 'ulaw':
            case 'mulaw':
                return 'pcm_mulaw';

            // A-law 编码
            case 'alaw':
                return 'pcm_alaw';

            // GSM（适用于极低带宽语音）
            case 'gsm':
                return 'libgsm';

            // iLBC（Internet Low Bitrate Codec）
            case 'ilbc':
                return 'ilbc'; // ⚠️ 并非所有 FFmpeg 编译启用 ilbc

            // 默认：如果找不到，尝试不转码
            default:
                return 'copy';
        }
    }


    /**
     * 停止音频发送
    */
    stop() {
        this.check_count = 0;
        this.accumulated_data = Buffer.alloc(0);
        clearTimeout(this.send_timer);
        this.send_timer = null;
        this.started = false;
        this.stoped = true;
        if (this.ws && this.ws._socket && this.ws._socket._writableState) {
            // 清空内部缓冲区
            this.ws._socket._writableState.buffer = [];
            this.ws._socket._writableState.length = 0;
        }
        this.inputStream && this.inputStream.destroy();
        this.outputStream && this.outputStream.destroy();
        if (this.ffmpegProcess && !this.ffmpegProcess.killed && this.ffmpegProcess.kill) {
            this.ffmpegProcess.kill('SIGKILL');
        }
        if (!G_devices.get(this.device_id)) return;
        G_devices.set(this.device_id, { ...G_devices.get(this.device_id), play_audio_ing: false, audio_sender: null, });

        // 关闭音频保存流（仅当保存功能启用时）
        if (this.audioSaver.isEnabled()) {
            this.audioSaver.toggle(false);
        }
    }
    /**
     * 获取缓冲区大小
    */
    getBufferSize() {
        return this.accumulated_data.byteLength;
    }
}
module.exports = Audio_sender;
