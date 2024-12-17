
class AudioSender {
    constructor(threshold, sendCallback) {
        this.threshold = threshold;   // 缓存的阈值，达到此值时发送数据
        this.cache = Buffer.alloc(0);  // 用于缓存接收到的音频数据
        this.sendCallback = sendCallback;  // 发送回调函数
    }

    // 接收数据并将其添加到缓存
    addData(chunk) {
        this.cache = Buffer.concat([this.cache, chunk]);

        // 如果缓存的大小达到阈值，执行发送操作
        if (this.cache.length >= this.threshold) {
            this.sendData();
        }
    }

    // 执行数据发送操作
    sendData() {
        // console.log('发送数据，大小:', this.cache.length); 
        this.sendCallback(this.cache); 
        this.cache = Buffer.alloc(0);
    }

    // 手动调用此方法来处理缓存中的剩余数据
    flushRemaining() {
        if (this.cache.length > 0) { 
            this.sendCallback(this.cache);
            this.cache = Buffer.alloc(0);
        }
    }
}

module.exports = AudioSender;
