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
