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


function add_audio_out_over_queue_hoc(audio_queue) {
    return (key, fn) => {
        if (!key) {
            log.error("add_audio_out_over_queue key is null");
            return;
        }
        const fns = audio_queue.get(key) || [];
        fns.push(fn)
        audio_queue.set(key, fns);
    }
}

function run_audio_out_over_queue_hoc(audio_queue) {
    return async (key) => {
        const fns = audio_queue.get(key);
        if (fns) {
            for (const fn of fns) {
                await fn();
            }
            audio_queue.delete(key);
        }
    }
}

function clear_audio_out_over_queue_hoc(audio_queue) {
    return () => audio_queue.clear()
}

module.exports = { add_audio_out_over_queue_hoc, run_audio_out_over_queue_hoc, clear_audio_out_over_queue_hoc }