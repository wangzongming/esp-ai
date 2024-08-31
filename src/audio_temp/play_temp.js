/**
 * Copyright (c) 2024 小明IO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Commercial use of this software requires prior written authorization from the Licensor.
 * 请注意：将 ESP-AI 代码用于商业用途需要事先获得许可方的授权。
 * 删除与修改版权属于侵权行为，请尊重作者版权，避免产生不必要的纠纷。
 * 
 * @author 小明IO   
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
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
                // 填充头信息: session_id   
                const sessionIdBuffer = Buffer.from("0000", 'utf-8');
                const combinedBuffer = Buffer.concat([sessionIdBuffer, chunk]);
                client_ws.send(combinedBuffer); 
            }
        });
        readStream.on('end', () => { 
            const endFlagBuf = Buffer.from("2001", 'utf-8');
            client_ws.send(endFlagBuf);
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