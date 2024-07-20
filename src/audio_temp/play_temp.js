const fs = require('fs');
const path = require('path');

function play_temp(name, client_ws) {
    return new Promise((resolve, reject) => {
        const filePath = path.join(__dirname, `${name}`);
        const readStream = fs.createReadStream(filePath);
        let isFirst = true;
        client_ws.send(JSON.stringify({ type: "play_audio", tts_task_id: "warning_tone" }));
        readStream.on('data', (audio) => {
            let c_l = isFirst ? G_max_audio_chunk_size * 2 : G_max_audio_chunk_size;
            isFirst = false;
            for (let i = 0; i < audio.length; i += c_l) {
                const end = Math.min(i + c_l, audio.length);
                const chunk = audio.slice(i, end);
                client_ws.send(chunk);
            }
        });
        readStream.on('end', () => {
            // setTimeout(() => {
            //     resolve(true);
            // }, 500)
            resolve(true);
        });
        readStream.on('error', (err) => {
            console.error('读取文件时出错:', err);
            reject(err)
        });
    });
}
module.exports = play_temp;