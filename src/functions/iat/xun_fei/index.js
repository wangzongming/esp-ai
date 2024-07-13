/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws') 
const getServerURL = require("../../getServerURL");
const TTS_FN = require(`../../tts`);

/**
 * 讯飞语音识别 
 * @param {String} device_id 设备id  
 * @param {Function} cb ({text, device_id})=> void 回调函数   
*/
function IAT_FN(device_id, cb) {
    const { devLog, api_key, iat_server } = G_config;
    devLog && console.log('\n=== 开始请求语音识别 ===');
    const config = {
        appid: api_key[iat_server].appid,
    }

    G_devices.set(device_id, {
        ...G_devices.get(device_id),
        iat_status: XF_IAT_FRAME.STATUS_FIRST_FRAME,
    })
    const iatResult = [];
    const iat_ws = new WebSocket(getServerURL("IAT"))
    G_devices.set(device_id, {
        ...G_devices.get(device_id),
        iat_status: XF_IAT_FRAME.STATUS_FIRST_FRAME,
        iat_ws: iat_ws,
        iat_server_connected: false,
    })

    // 长时间无反应时应该自动关闭
    let close_connect_timer = null;
    // 连接建立完毕，读取数据进行识别
    iat_ws.on('open', (event) => {
        devLog && console.log("-> 讯飞 IAT 服务连接成功!")
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            iat_server_connected: true,
        })
        clearTimeout(close_connect_timer);
        close_connect_timer = setTimeout(() => {
            iat_ws && iat_ws.close();
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_ws: null,
                first_session: true,
                iat_server_connected: false,
            })
            devLog && console.log('\n\n=== 会话结束 ===\n\n')
        }, 2500);
    })

    let realStr = "";
    // 得到识别结果后进行处理，仅供参考，具体业务具体对待
    iat_ws.on('message', (data, err) => {
        clearTimeout(close_connect_timer);
        if (err) {
            console.log(`err:${err}`)
            return
        }

        res = JSON.parse(data)
        if (res.code != 0) {
            console.log(`error code ${res.code}, reason ${res.message}`)
            return
        }

        let str = ""
        if (res.data.status == 2) {
            iat_ws.close();
            clearTimeout(close_connect_timer);
            G_devices.set(device_id, {
                ...G_devices.get(device_id),
                iat_server_connected: false,
                iat_ws: null,
            })

            // res.data.status ==2 说明数据全部返回完毕，可以关闭连接，释放资源 
            currentSid = res.sid
            iatResult.forEach(i => {
                if (i != null) {
                    i.ws.forEach(j => {
                        j.cw.forEach(k => {
                            realStr += k.w;
                        })
                    })
                }
            })

            devLog && console.log(`-> 最终识别结果：${realStr}`) 
            cb({ text: realStr, device_id });
            return;
        } else {
            str += "-> 中间识别结果"
        }
        iatResult[res.data.result.sn] = res.data.result
        if (res.data.result.pgs == 'rpl') {
            res.data.result.rg.forEach(i => {
                iatResult[i] = null
            })
            str += "【动态修正】"
        }
        str += "："
        iatResult.forEach(i => {
            if (i != null) {
                i.ws.forEach(j => {
                    j.cw.forEach(k => {
                        str += k.w
                    })
                })
            }
        })
        devLog && console.log(str)
    })

    // 资源释放
    iat_ws.on('close', () => {
        // 不等待 close 事件，直接释放资源

        // clearTimeout(close_connect_timer);
        // devLog && console.log('-> 语音识别服务断开!')
        // G_devices.set(device_id, {
        //     ...G_devices.get(device_id),
        //     iat_ws: null,
        //     iat_server_connected: false,
        // })
    })

    // 建连错误
    iat_ws.on('error', (err) => {
        clearTimeout(close_connect_timer);
        console.log("iat websocket connect err: " + err)
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            iat_ws: null,
            iat_server_connected: false,
        })
        // xfyunTts("语音识别失败，请重新试试。", true)
        TTS_FN(device_id, {
            text: "语音识别发生了错误。",
            reRecord: true,
            pauseInputAudio: true
        });
    })


    function send_pcm(data) {
        const { iat_server_connected, iat_status, iat_ws } = G_devices.get(device_id);
        if (!iat_server_connected) return;
        let frame = "";
        let frameDataSection = {
            "status": iat_status,
            // 这里的帧率一定要和 inmp441 终端对上
            "format": "audio/L16;rate=16000",
            "audio": data.toString('base64'),
            "encoding": "raw"
        }
        switch (iat_status) {
            case XF_IAT_FRAME.STATUS_FIRST_FRAME:
                frame = {
                    // 填充common
                    common: {
                        app_id: config.appid
                    },
                    // 填充business
                    business: {
                        vad_eos: 3000,
                        language: "zh_cn",
                        domain: "iat",
                        accent: "mandarin",
                        dwa: "wpgs" // 可选参数，动态修正
                    },
                    //填充data
                    data: frameDataSection
                }
                G_devices.set(device_id, {
                    ...G_devices.get(device_id),
                    iat_status: XF_IAT_FRAME.STATUS_CONTINUE_FRAME,
                })
                break;
            case XF_IAT_FRAME.STATUS_CONTINUE_FRAME:
            case XF_IAT_FRAME.STATUS_LAST_FRAME:
                //填充frame
                frame = {
                    data: frameDataSection
                }
                break;
        }
        iat_ws.send(JSON.stringify(frame))
    }

    G_devices.set(device_id, {
        ...G_devices.get(device_id),
        send_pcm: send_pcm
    })
}

module.exports = IAT_FN;