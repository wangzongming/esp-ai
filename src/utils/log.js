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

const colors = require('colors');
const moment = require('moment');
function info(text, styles = []) {
    const { logs = {}} = G_config;
    const bold = styles?.includes?.('bold');
    text = text.cyan;
    if (bold) { text = text.bold }
    if (logs.info) {
        logs.info(text);
        return;
    };
    console.log(text);
}

function time() {
    return moment().format("HH:mm:ss SSS");
}
function t_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [Info] ${text.join(' ')}`.cyan);
        return;
    };
    console.log(`${time()} [Info] ${text.join(' ')}`.cyan);
}
function t_red_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [Info] ${text.join(' ')}`.magenta);
        return;
    };
    console.log(`${time()} [Info] ${text.join(' ')}`.magenta);
}
function tts_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [TTS] ${text.join(' ')}`.green);
        return;
    };
    console.log(`${time()} [TTS] ${text.join(' ')}`.green);
}

function iat_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [IAT] ${text.join(' ')}`.blue);
        return;
    };
    console.log(`${time()} [IAT] ${text.join(' ')}`.blue);
}


function llm_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [LLM] ${text.join(' ')}`.yellow);
        return;
    };
    console.log(`${time()} [LLM] ${text.join(' ')}`.yellow);
}

function error(text, styles = []) {
    const { logs = {} } = G_config;
    const bold = styles?.includes?.('bold');
    text = text.red;
    if (bold) { text = text.bold }

    if (logs.info) {
        logs.error(`${time()} ❌ [Error] ${text}`);
        return;
    };
    console.log(`❌ ${text}`);
}
 
module.exports = {
    info,
    t_info,
    t_red_info,
    error,
    tts_info,
    iat_info,
    llm_info
}