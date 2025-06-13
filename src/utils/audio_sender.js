const log = require("./log.js");

/**
 * 音频流发送器 
 * 发送速率为 3kb~20kb/100ms~200ms = 10kb/1s
*/
class Audio_sender {
    constructor(ws, device_id) {
        this.ws = ws;
        this.send_timer = null;
        this.accumulated_data = Buffer.alloc(0);
        this.check_count = 0;
        this.started = false;
        this.stoped = false;
        this.device_id = device_id;
        // 发送速率，单位ms  
        this.send_speed = 500;
        // 发送流的大小，动态调整
        this.send_num = 1024 * 3;
        // 动态大小
        this.add_count = 1;

        // 客户端最大的缓存能力 字节流。 
        this.client_max_available_audio = 1024 * 20;
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
                    if (this.check_count >= 20) {
                        clearInterval(this.send_timer);
                        this.started = false;
                        this.send_timer = null;
                    }


                    this.check_count++;
                    this.send_timer = null;
                    setTimeout(this.sender, 80);
                    return;
                };

                if (!G_devices.get(this.device_id)) return;
                if (this.stoped) return;
                // 流量控制 
                let send_num = this.send_num * this.add_count;
                let send_speed = this.send_speed;
                const max = this.client_max_available_audio / 2;
                const { client_available_audio = 0 } = G_devices.get(this.device_id);
                if (client_available_audio < this.client_max_available_audio) {
                    if (send_num < max) {
                        this.add_count += 1;
                        send_speed = 80;
                    }
                } else {
                    if (this.add_count > 1) {
                        this.add_count -= 1;
                        send_speed = this.send_speed;
                    }
                }

                if (client_available_audio > this.client_max_available_audio) {
                    setTimeout(this.sender, 5);
                    return;
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
                const is_end = [G_session_ids["tts_all_end_align"], G_session_ids["tts_all_end"], G_session_ids["tts_chunk_end"]].includes(end_data_str);
                if (is_end) {
                    ss = end_data_str;
                    // 删除最后两个字节 
                    real_data = data.slice(0, data_len - 2);
                }
                // log.tts_info("发送-> 会话ID:", session_id, " 会话状态:", ss, " 数据长度:", real_data.length, " 客户端有效音频:", this.client_max_available_audio, this.stoped);
                if (!session_id) return log.error(`缺失会话ID，发送失败。`);

                const combinedBuffer = Buffer.concat([Buffer.from(session_id, 'utf-8'), Buffer.from(ss, 'utf-8'), real_data]);
                this.ws.send(combinedBuffer, () => {
                    clearTimeout(this.send_timer);
                    this.send_timer = setTimeout(this.sender, send_speed);
                    this.accumulated_data = this.accumulated_data.slice(data_len);
                    if (is_end) {
                        on_end && on_end();
                        resolve();
                    }
                });
            };
            this.sender();
        })
    }

    /**
     * 发送音频函数 
     */
    sendAudio(stream_chunk) {
        this.accumulated_data = Buffer.concat([this.accumulated_data, stream_chunk]);
    }

    /**
     * 停止音频发送
    */
    stop() { 
        this.check_count = 0;
        this.accumulated_data = Buffer.alloc(0);
        clearInterval(this.send_timer);
        this.send_timer = null;
        this.started = false;
        this.stoped = true;
        if (this.ws._socket && this.ws._socket._writableState) {
            // 清空内部缓冲区
            this.ws._socket._writableState.buffer = [];
            this.ws._socket._writableState.length = 0;
        } 
        if (!G_devices.get(this.device_id)) return;
        G_devices.set(this.device_id, { ...G_devices.get(this.device_id), play_audio_ing: false, audio_sender: null, });
    }
    /**
     * 获取缓冲区大小
    */
    getBufferSize() {
        return this.accumulated_data.byteLength;
    }
}
module.exports = Audio_sender;