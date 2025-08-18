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

const log = require("./utils/log");
const gen_intention_prompt = require("./utils/gen_intention_prompt");

const matchIntention = require("./instrance_fns/matchIntention")
const llm = require("./instrance_fns/llm")
const tts = require("./instrance_fns/tts")
const stop = require("./instrance_fns/stop")
const newSession = require("./instrance_fns/newSession")
const pinMode = require("./instrance_fns/pinMode")
const LEDCInit = require("./instrance_fns/LEDCInit")
const ledcWrite = require("./instrance_fns/ledcWrite")
const digitalWrite = require("./instrance_fns/digitalWrite")
const digitalRead = require("./instrance_fns/digitalRead")
const analogWrite = require("./instrance_fns/analogWrite")
const analogRead = require("./instrance_fns/analogRead")
const { reCache } = require("./functions/client_messages")
const isSpeaking = require("./instrance_fns/isSpeaking")
const awaitPlayerDone = require("./instrance_fns/awaitPlayerDone")
const awaitIntention = require("./instrance_fns/awaitIntention")

class EspAiInstance {
    constructor() { }

    /**
     * 获取连接了的所有设备, 或者指定设备ID的设置
    */
    getClients(device_id) {
        return device_id ? G_devices.get(device_id) : G_devices;
    }

    /**
     * 服务端设置客户端端wifi信息的方法, 返回设置结果
     * 设置客户端 wifi 信息和存贮的业务数据，也就是配网页面设置的值，都可以用这个方法来改
     * 等同于硬件端的 .setWifiConfig 方法
     * @params device_id 指定设备
     * @params arg 可该项：wifi_name | wifi_pwd | api_key | ext1 | ext2 | ext3 | ext4 | ext5 | ext6 | ext7
    */
    setWifiConfig(device_id, arg) {
        !device_id && log.error(`调用 setWifiConfig 方法时，请传入 device_id`);
        return new Promise((resolve) => {
            if (!G_devices.get(device_id)) return;
            const { ws } = G_devices.get(device_id);
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                set_wifi_config_end: resolve,
            })
            ws && ws.send(JSON.stringify({ type: "set_wifi_config", configs: arg }));
        })
    }

    /**
     * 更新客户端配置项也就是 gen_client_config 配置返回出来的数据
    */
    updateClientConfig(device_id, config) {
        !device_id && log.error(`调用 updateClientConfig 方法时，请传入 device_id`);
        const oldConfig = G_devices.get(device_id);
        if (oldConfig) {
            // 缓存信息不可在此处清除，本函数仅仅用于更新配置
            const new_config = {
                ...oldConfig,
                user_config: {
                    ...oldConfig.user_config,
                    ...config,
                },
            };
            G_devices.set(device_id, {
                ...new_config,
                intention_prompt: gen_intention_prompt(new_config.user_config.intention),
            })
        }
    }

    /**
     * 清除客户端音频缓存
    */
    reCache(device_id) {
        !device_id && log.error(`调用 reCache 方法时，请传入 device_id`);
        const { ws: ws_client } = G_devices.get(device_id);
        ws_client && ws_client.send(JSON.stringify({ type: "clear_cache" }), () => reCache({ device_id }));
    }

    llm = llm;
    tts = tts;
    stop = stop;
    newSession = newSession;
    matchIntention = matchIntention;
    pinMode = pinMode;
    LEDCInit = LEDCInit;
    ledcWrite = ledcWrite;
    digitalWrite = digitalWrite;
    digitalRead = digitalRead;
    analogWrite = analogWrite;
    analogRead = analogRead;
    isSpeaking = isSpeaking;
    awaitPlayerDone = awaitPlayerDone;
    awaitIntention = awaitIntention;

    /**
     *  重启设备
    */
    async restart(device_id) {
        !device_id && log.error(`调用 restart 方法时，请传入 device_id`);
        if (!G_devices.get(device_id)) return;
        const { ws: ws_client } = G_devices.get(device_id);
        ws_client && ws_client.send(JSON.stringify({ type: "restart" }));
    }

    /**
     *  手动设置设备本地存储的数据，值为空字符串时为清空
     *  和 setWifiConfig 的区别是：本函数可将值设置为空字符串， setWifiConfig 为批量更新，空字符串会直接省略
    */
    async setLocalData(device_id, field, value) {
        !device_id && log.error(`调用 setLocalData 方法时，请传入 device_id`);
        if (!G_devices.get(device_id)) return;
        const { ws: ws_client } = G_devices.get(device_id);
        ws_client && ws_client.send(JSON.stringify({
            type: "set_local_data",
            field,
            value
        }));
    }

    /**
     * 设置用户的上下文，当对话存在多角色时需要在业务代码中自行调用本方法进行切换会话
     * @param llm_historys {"role": "user" | "assistant" | "system", "content":string}[]
    */
    setLLMHistorys(device_id, llm_historys = []) {
        !device_id && log.error(`调用 setLLMHistorys 方法时，请传入 device_id`);
        if (!G_devices.get(device_id)) return;
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            llm_historys,
        })
    }

    /**
     * 获取用户的上下文，在设置上下文时一般需要将当前上下文先存起来，否则切换回来时会丢失
     * @return llm_historys {"role": "user" | "assistant" | "system", "content":string}[]
    */
    getLLMHistorys(device_id) {
        !device_id && log.error(`调用 getLLMHistorys 方法时，请传入 device_id`);
        if (!G_devices.get(device_id)) return;
        const { llm_historys } = G_devices.get(device_id);
        return llm_historys
    }
}

module.exports = EspAiInstance;
