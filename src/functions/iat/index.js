/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
async function cb({ device_id, text }) {
    const { devLog, intention = [], onIATcb, sleep_reply } = G_config;
    const TTS_FN = require(`../tts`);
    const LLM_FN = require(`../llm`);
    onIATcb && onIATcb({ device_id, text });
    const { first_session, ws: ws_client } = G_devices.get(device_id);
    // 调用聊天模型
    if (text.length) {
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            first_session: false,
        })

        let task_info = null;
        intention_for: for (const item of intention) {
            const { key = [] } = item;
            // 后期需要加入意图识别模型...
            if (key.includes(text)) {
                task_info = item;
                break intention_for;
            }
        }
        if (task_info) {
            const { instruct, message } = task_info;
            switch (instruct) {
                case "__sleep__":
                    await TTS_FN(device_id, {
                        text: message || sleep_reply || "我先退下了，有需要再叫我。",
                        reRecord: false,
                        pauseInputAudio: true
                    });
                    G_devices.set(device_id, {
                        ...G_devices.get(device_id),
                        first_session: true,
                    })
                    ws_client && ws_client.send("session_end");
                    ws_client && ws_client.send("iat_end");
                    devLog && console.log('\n\n === 会话结束 ===\n\n')
                    break;
                default:
                    instruct && ws_client && ws_client.send(instruct);
                    message && await TTS_FN(device_id, {
                        text: message,
                        reRecord: true,
                        pauseInputAudio: true
                    });
                    break;
            }
        } else {

            // 其他情况交给 LLM
            LLM_FN(device_id, { text })
        }
    } else {
        !first_session && TTS_FN(device_id, {
            text: sleep_reply || "我先退下了，有需要再叫我。",
            reRecord: false,
            pauseInputAudio: true
        });
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            first_session: true,
        })
        ws_client && ws_client.send("session_end");
        ws_client && ws_client.send("iat_end");
    }
}
module.exports = (device_id) => {
    const { iat_server } = G_config;
    const IAT_FN = require(`./${iat_server}`);
    return IAT_FN(device_id, cb)
};
