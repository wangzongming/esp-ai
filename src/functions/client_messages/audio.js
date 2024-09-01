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

/**
 * 客户端音频数据处理
*/
const { iat_info } = require("../../utils/log");

// 模拟音频测试压力 
// const fs = require('fs');
// const path = require('path');
// const writeStream = fs.createWriteStream(path.join(__dirname, './output.bin'));
// let timer = null;

function fn({ device_id, data }) {
    // 压力测试数据摘取  
    // writeStream.write(data);
    // clearTimeout(timer);
    // timer = setTimeout(() => {
    //     writeStream.end();
    // }, 1000)

    try {
        const { devLog, onIATEndcb } = G_config;
        const { started, send_pcm, iat_server_connected, iat_end_frame_timer, iat_end_queue, user_config } = G_devices.get(device_id);
        // console.log("---", started, data.length)
        if (started && data && data.length && send_pcm && iat_server_connected) {
            // 发送数据 
            send_pcm(data);
            // 准备发送最后一帧
            clearTimeout(iat_end_frame_timer);
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_end_frame_timer: setTimeout(async () => {
                    const { iat_server_connected } = G_devices.get(device_id);
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        started: false,
                    })
                    if (!iat_server_connected) {
                        return;
                    }
                    devLog && iat_info("IAT 超时未收到音频数据，执行主动结束回调。");
                    iat_end_queue && await iat_end_queue();
                    onIATEndcb && await onIATEndcb(device_id);
                }, (user_config.iat_config.vad_eos || 2500) - 300) // 需要比静默时间少,
            })
        }
    } catch (err) {
        console.log(err);
        log.error(`音频 消息错误： ${err}`)
    }
}

module.exports = fn