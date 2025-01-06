
const path = require('path');
const fs = require('fs');
const log = require('../utils/log');
const play_temp = require('./play_temp');
const fn = (ws) => {
    const { iatDu } = G_config;
    const filePath = iatDu === true ? path.join(__dirname, `./du.mp3`) : iatDu;
    if (!fs.existsSync(filePath)) {
        log.error(`提示音地址未找到音频文件，请正确配置 iatDu ！！！`)
    }
    return async () => {
        // await play_temp("du.pcm", ws, 0.8, 24);
        await play_temp(filePath, ws);
    }
}
module.exports = fn;