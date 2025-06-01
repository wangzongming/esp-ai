
const path = require('path');
const fs = require('fs');
const log = require('../utils/log');
const fn = (ws) => {
    const { iatDu = false } = G_config;
    if (iatDu !== false) {
        const filePath = iatDu === true ? path.join(__dirname, `./du.mp3`) : iatDu;
        if (!fs.existsSync(filePath)) {
            log.error(`提示音地址未找到音频文件，请正确配置 iatDu ！！！`)
        }
        const readStream = fs.createReadStream(filePath, {});
        readStream.on('data', (audio) => { 
            const combinedBuffer = Buffer.concat([
                Buffer.from(G_session_ids.cache_du, 'utf-8'),
                Buffer.from(G_session_ids.tts_session, 'utf-8'),
                audio]);
            ws.send(combinedBuffer);
        });
        readStream.on('end', () => {
            log.t_info(`提示音缓存完毕`)
            const combinedBuffer = Buffer.concat([
                Buffer.from(G_session_ids.cache_du, 'utf-8'),
                Buffer.from(G_session_ids.tts_all_end, 'utf-8'),
            ]);
            ws.send(combinedBuffer);
        });
        readStream.on('error', (err) => {
            console.error('读取提示音文件时出错:', err);
        });
    }
}
module.exports = fn;