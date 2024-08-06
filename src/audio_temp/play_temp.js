/** 
 * 请注意保留版权
 * @author 小明IO 
 * @github https://github.com/wangzongming/esp-ai  
 */

const fs = require('fs');
const path = require('path'); 

/**
 * @volumeFactor 调整音量的系数，0.5 表示减半
 * @abit         PCM 数据是 16 位 (2 字节) 小端格式
 * */ 
function play_temp(name, client_ws, volumeFactor = 1, abit = 2) { 
    return new Promise((resolve, reject) => {
        const filePath = path.join(__dirname, `${name}`);
        const readStream = fs.createReadStream(filePath);
        let isFirst = true;
        client_ws.send(JSON.stringify({ type: "play_audio", tts_task_id: "warning_tone" }));
        readStream.on('data', (audio) => {  
            const adjustedAudio = Buffer.alloc(audio.length);

            //  PCM 数据是 16 位 (2 字节) 小端格式
            for (let i = 0; i < audio.length; i += abit) {
                const sample = audio.readInt16LE(i);
                const adjustedSample = Math.floor(sample * volumeFactor);
                adjustedAudio.writeInt16LE(adjustedSample, i);
            }

            let c_l = isFirst ? G_max_audio_chunk_size * 2 : G_max_audio_chunk_size;
            isFirst = false;
            for (let i = 0; i < adjustedAudio.length; i += c_l) {
                const end = Math.min(i + c_l, adjustedAudio.length);
                const chunk = adjustedAudio.slice(i, end);
                client_ws.send(chunk);
            }
        });
        readStream.on('end', () => { 
            client_ws.send(JSON.stringify({ type: "tts_send_end", tts_task_id: "warning_tone" }));
            resolve(true);
        });
        readStream.on('error', (err) => {
            console.error('读取文件时出错:', err);
            reject(err)
        });
    });
}
module.exports = play_temp;