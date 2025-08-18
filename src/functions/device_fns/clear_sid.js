
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
 * 清空会话ID
*/
function clear_sid(device_id) {
    // const now = new Date();
    // // 生成 ISO 格式时间（2025-07-25T14:17:43.028）
    // const ios = now.toISOString().replace('Z', '');
    // // 提取时分秒（14:17:43）
    // const hms = now.toTimeString().split(' ')[0];
    // // 提取毫秒部分（028）
    // const ms = now.getMilliseconds().toString().padStart(3, '0');
    // const time = `${hms} ${ms}`;
    // console.log(`${ios}\x1b[1m\x1b[36m ${time} [Info] ------------------------- 开始清理会话数据 -------------------------- \x1b[0m`);
    // const error = new Error();
    // const stack = error.stack.split('\n');
    // // stack[0]: "Error" stack[1]: 当前函数 stack[2]: 调用者
    // console.log(`${ios}\x1b[1m\x1b[36m ${time} [Info] 调用者: ${(stack[2] || '未知调用者').trim()} \x1b[0m`);

    G_devices.set(device_id, { ...G_devices.get(device_id), session_id: "" });
}
module.exports = clear_sid
