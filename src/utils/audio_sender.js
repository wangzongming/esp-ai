/**
 * 音频流发送器
*/
class Audio_sender {
    constructor(ws, device_id) {
        this.ws = ws;
        this.send_timer = null;
        this.accumulated_data = Buffer.alloc(0);
        this.send_num = G_max_audio_chunk_size;
        this.check_count = 0;
        this.started = false;
        this.device_id = device_id; 
    }

    /**
     * 启动音频发送
    */
    startSend(session_id, on_end) {
        return new Promise((resolve) => {
            this.started = true;
            this.check_count = 0;
            this.accumulated_data = Buffer.alloc(0);

            this.sender = () => {
                if (!this.accumulated_data.length) {
                    if (this.check_count >= 20) {
                        clearInterval(this.send_timer);
                        this.started = false;
                        this.send_timer = null;
                    }
                    this.check_count++;
                    this.send_timer = null;
                    setTimeout(this.sender, 350)
                    return;
                };

                let data = null;
                const remain = this.accumulated_data.length - this.send_num;
                if (remain > 0 && remain < 4) { 
                    data = this.accumulated_data;
                } else {
                    data = this.accumulated_data.slice(0, this.send_num);
                }
                const  { session_id: now_session_id } = G_devices.get(this.device_id); 
                if(now_session_id && session_id && now_session_id !== session_id) return;
                
                const _session_id = session_id ? `${session_id}` : "0000";
                const end_str = data.slice(-4).toString();
                if ([G_session_ids["tts_all_end_align"], G_session_ids["tts_all_end"], G_session_ids["tts_chunk_end"]].includes(end_str)) {
                    // 删除最后四个字节 
                    const real_data = data.slice(0, data.length - 4);
                    const end_data = data.slice(-4);
                    const combinedBuffer = Buffer.concat([Buffer.from(_session_id, 'utf-8'), real_data]); 
 
                    this.ws.send(combinedBuffer, () => { 
                        this.ws.send(end_data, () => {
                            this.send_timer = setTimeout(this.sender, 350)
                            this.accumulated_data = this.accumulated_data.slice(data.length);
                            on_end && on_end();
                            resolve();
                        });
                    });
                } else { 
                    const combinedBuffer = Buffer.concat([Buffer.from(_session_id, 'utf-8'), data]);
                    this.ws.send(combinedBuffer, () => {
                        this.send_timer = setTimeout(this.sender, 350)
                        this.accumulated_data = this.accumulated_data.slice(data.length);
                    });
                } 
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
    }
}
module.exports = Audio_sender;