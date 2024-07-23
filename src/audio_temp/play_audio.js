
const https = require('https');
const http = require('http');
const log = require('../utils/log');
const ffmpeg = require('fluent-ffmpeg');
const ffmpegPath = require('ffmpeg-static');
const { PassThrough } = require('stream');

function isHttpUrl(url) {
    const regex = /^https?:\/\/.+/;
    return regex.test(url);
}

function isHttpsUrl(url) {
    const regex = /^https:\/\/.+/;
    return regex.test(url);
}

function urlToStream(url) {
    const stream = new PassThrough();
    const hf = isHttpsUrl(url) ? https : http;
    hf.get(url, (response) => {
        if (response.statusCode !== 200) {
            log.error(`音频地址不可用：${url}`)
            stream.emit('error', new Error(`Request failed with status code ${response.statusCode}`));
            return;
        }

        response.pipe(stream);
    }).on('error', (err) => {
        stream.emit('error', err);
    });

    return stream;
}
function convertStream(inputStream) {
    const outputStream = new PassThrough();

    ffmpeg(inputStream)
        .setFfmpegPath(ffmpegPath)
        .format('s16le')
        .audioFrequency(16000)
        .audioChannels(1)
        .on('error', (err) => {
            outputStream.emit('error', err);
        })
        .pipe(outputStream);

    return outputStream;
}

/**
 * 播放任何 mp3、wav 的声音
 * 提供 http 地址
*/
function play_audio(url, client_ws, task_id) {
    return new Promise((resolve, reject) => {
        let real_url = "";
        if (isHttpUrl(url)) {
            real_url = url;
        } else {
            // real_url = path.join(__dirname, `${url}`);
            log.error("play_audio 不支持本地地址！")
        }

        client_ws.send(JSON.stringify({ type: "play_audio", tts_task_id: task_id || "any_audio" }));

        const inputStream = urlToStream(url);
        const outputStream = convertStream(inputStream);

        let isFirst = true;
        outputStream.on('data', (audio) => {
            let c_l = isFirst ? G_max_audio_chunk_size * 2 : G_max_audio_chunk_size;
            isFirst = false;
            for (let i = 0; i < audio.length; i += c_l) {
                const end = Math.min(i + c_l, audio.length);
                const chunk = audio.slice(i, end);
                client_ws.send(chunk);
            }
        });
        outputStream.on('end', () => {
            resolve(true);
        });
        outputStream.on('error', (err) => {
            log.error(`Stream error: ${err.message}`);
            reject(err)
        });
    });
}
module.exports = play_audio;