/**
 * 音频流发送器
*/
class Audio_sender {
    constructor(ws) {
        this.ws = ws;
        this.send_timer = null;
        this.accumulated_data = Buffer.alloc(0);
        this.send_num = G_max_audio_chunk_size;
        this.check_count = 0;
        this.started = false;
    }

    /**
     * 启动音频发送
    */
    startSend(session_id, on_end) {
        return new Promise((resolve) => {
            this.started = true;
            this.check_count = 0;
            this.accumulated_data = Buffer.alloc(0);
            clearInterval(this.send_timer);
            this.send_timer = setInterval(() => {
                if (!this.accumulated_data.length) {
                    if (this.check_count >= 20) {
                        clearInterval(this.send_timer);
                        this.started = false;
                        this.send_timer = null;
                    }
                    this.check_count++;
                    this.send_timer = null;
                    return;
                };
 
                let data = null;
                const remain = this.accumulated_data.length - this.send_num;
                if (remain > 0 && remain < 4) { 
                    data = this.accumulated_data;
                } else {
                    data = this.accumulated_data.slice(0, this.send_num);
                }
                // session_id
                const _session_id = session_id ? `${session_id}` : "0000";
 
                const end_str = data.slice(-4).toString();
                if ([G_session_ids["tts_all_end_align"], G_session_ids["tts_all_end"], G_session_ids["tts_chunk_end"]].includes(end_str)) {
                    // console.log("执行结束回调：")
                    // 删除最后四个字节
                    const end_data = data.slice(0, -4);
                    end_data.length && this.ws.send(end_data);
                    // 特殊发一组标志
                    this.ws.send(Buffer.from(end_str, 'utf-8'));
                    on_end && on_end();
                    resolve();
                } else {
                    const sessionIdBuffer = Buffer.from(_session_id, 'utf-8');
                    const combinedBuffer = Buffer.concat([sessionIdBuffer, data]);
                    this.ws.send(combinedBuffer);
                } 
                this.accumulated_data = this.accumulated_data.slice(data.length);
            }, 300);// 给网络延时预留一定的时间
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