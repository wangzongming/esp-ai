 
/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const WebSocket = require('ws')
const getServerURL = require("../../getServerURL");

/**
 * 讯飞语音识别  
 * @param {String}      device_id           设备ID    
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志   
 * @param {Object}      api_key             用户配置的key   
 * @param {Number}      vad_eos             用户配置的静默时间，超过这个时间不说话就结束语音识别  
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> {} })
 * @param {Function}    iatServerErrorCb    与 TTS 服务之间发生错误时调用，并且传入错误说明，eg: ttsServerErrorCb("意外错误") 
 * @param {Function}    cb                  IAT 识别的结果调用这个方法回传给框架 eg: cb({ text: "我是语音识别结果"  })
 * @param {Function}    logSendAudio        记录发送音频数据给服务的函数，框架在合适的情况下会进行调用
 * @param {Function}    connectServerCb     连接 iat 服务后需要调用这个方法告诉框架：eg: connectServerCb(true)
 * @param {Function}    serverTimeOutCb     当 IAT 服务连接成功了，但是长时间不响应时
 * @param {Function}    iatEndQueueCb       iat 静默时间达到后触发， 一般在这里面进行最后一帧的发送，告诉服务端结束识别 
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.iat_info("iat 专属信息")
 * 
 *  
*/
function IAT_FN({ device_id, log, devLog, api_key, vad_eos, cb, iatServerErrorCb, logWSServer, logSendAudio, connectServerCb, serverTimeOutCb, iatEndQueueCb }) {
    const config = { ...api_key }

    const iatResult = [];
    const iat_ws = new WebSocket(getServerURL("IAT"))
    logWSServer(iat_ws);

    // 讯飞 IAT 帧定义
    const XF_IAT_FRAME = {
        STATUS_FIRST_FRAME: 0,
        STATUS_CONTINUE_FRAME: 1,
        STATUS_LAST_FRAME: 2
    }
    let iat_server_connected = false;
    let iat_status = XF_IAT_FRAME.STATUS_FIRST_FRAME;



    // 长时间无反应时应该自动关闭
    let close_connect_timer = null;
    // 连接建立完毕，读取数据进行识别
    iat_ws.on('open', (event) => {
        devLog && log.iat_info("-> 讯飞 IAT 服务连接成功!")
        iat_server_connected = true;
        connectServerCb(true);

        clearTimeout(close_connect_timer);
        close_connect_timer = setTimeout(() => {
            serverTimeOutCb();
        }, 5000);
    })

    // 当达到静默时间后会自动执行这个任务
    iatEndQueueCb(() => {
        clearTimeout(close_connect_timer);
        if (iat_server_connected && send_pcm) {
            iat_status = XF_IAT_FRAME.STATUS_LAST_FRAME;
            send_pcm("");
        }
    })

    let realStr = "";
    // 得到识别结果后进行处理，仅供参考，具体业务具体对待
    iat_ws.on('message', (data, err) => {
        clearTimeout(close_connect_timer);
        if (err) {
            log.iat_info(`err:${err}`)
            return
        }

        res = JSON.parse(data)
        if (res.code != 0) {
            log.iat_info(`error code ${res.code}, reason ${res.message}`)
            return
        }

        let str = ""
        if (res.data.status == 2) {
            iat_ws.close(); 

            iat_server_connected = false;
            connectServerCb(false);

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

            devLog && log.iat_info(`-> 最终识别结果：${realStr}`)
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
        devLog && log.iat_info(str)
    })

    // 资源释放
    iat_ws.on('close', () => {
        devLog && log.iat_info("-> 讯飞 IAT 服务已关闭")
        clearTimeout(close_connect_timer); 
        iat_server_connected = false;
        connectServerCb(false);
    })

    // 建连错误
    iat_ws.on('error', (err) => {
        clearTimeout(close_connect_timer);
        iatServerErrorCb(err);
        iat_server_connected = false;
        connectServerCb(false);
    })


    function send_pcm(data) {
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
                        vad_eos: vad_eos,
                        language: "zh_cn",
                        domain: "iat",
                        accent: "mandarin",
                        dwa: "wpgs" // 可选参数，动态修正
                    },
                    //填充data
                    data: frameDataSection
                }
                iat_status = XF_IAT_FRAME.STATUS_CONTINUE_FRAME;
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

    logSendAudio(send_pcm)
}

module.exports = IAT_FN;