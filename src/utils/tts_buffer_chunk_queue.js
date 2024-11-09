
class TTS_buffer_chunk_queue {
    constructor(device_id) {
        this.device_id = device_id;
        this.queue = [];
        this.stoped = true;
        this.runing = false;
        this.queue_listen_timer = true; 
    } 
    push(args) { 
        this.queue.push(args); 
        !this.runing && this.run();
    }
    async run() {
        if (!this.queue.length) {
            this.stoped = true;
            this.runing = false;
            return;
        } 
        this.runing = true;
        const tts_queue = this.queue.shift();
        await tts_queue();
        this.run(); 
    } 
    clear() {
        this.runing = false;
        this.stoped = true;
        this.queue = [];
    }
}
module.exports = TTS_buffer_chunk_queue;