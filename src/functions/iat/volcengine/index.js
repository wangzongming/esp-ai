
/*
 * MIT License
 *
 * Copyright (c) 2025-至今 小明IO
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @author 小明IO
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */
const WebSocket = require('ws');
const zlib = require('zlib');
const { v4: uuidv4 } = require('uuid');
const fs = require('fs');
const path = require('path');

// 默认 WebSocket 消息头（4 字节）
const DefaultFullClientWsHeader = Buffer.from([0x11, 0x10, 0x11, 0x00]);
const DefaultAudioOnlyWsHeader = Buffer.from([0x11, 0x20, 0x11, 0x00]);
const DefaultLastAudioWsHeader = Buffer.from([0x11, 0x22, 0x11, 0x00]);

// 消息类型常量
const SERVER_FULL_RESPONSE = 0x09; // 二进制 1001
const SERVER_ACK = 0x0B; // 二进制 1011
const SERVER_ERROR_RESPONSE = 0x0F; // 二进制 1111

// 压缩与序列化标识
const GZIP = 0x01;
const JSON_TYPE = 0x01;


class volcengineAsrClient {
    constructor({ url, appid, token, cluster, uid, getClientAudioConfig }) {
        const { sample_rate = 16000, channels = 1, format = "pcm", language = "" } = getClientAudioConfig("volcengine");
        this.appid = appid;
        this.token = token;
        this.cluster = cluster;
        // 固定的工作流
        this.workflow = "audio_in,resample,partition,vad,fe,decode";
        // 默认音频格式与编解码方式
        this.channel = channels;
        this.rate = sample_rate;
        
        // this.format = "mp3";
        this.format =  format;
        this.codec =  "";
        if(format === "pcm"){ 
            this.codec = "raw";  
        }
        if(format === "ogg" || format === "opus"){ 
            this.codec = "opus";  
        }

        this.url = url || "wss://openspeech.bytedance.com/api/v2/asr";
        this.onOpen = null;
        this.onMessage = null;
        this.onError = null;
        this.onClose = null;
        this.uid = uid;

        // 全部音频 
        this.audioBuffers = [];
        this.sendTimer = null;

        this.ws = new WebSocket(this.url, {
            headers: {
                'Authorization': `Bearer;${this.token}`
            }
        });

        // test...
        // this.writeStreamMP3 = fs.createWriteStream(path.join(__dirname, `./test.pcm`));

        this.ws.on('open', async () => {
            this.onOpen && this.onOpen();
            const reqBuffer = this.constructRequest();
            const compressedReq = this.gzipCompress(reqBuffer);
            const payloadSizeBuffer = Buffer.alloc(4);
            payloadSizeBuffer.writeUInt32BE(compressedReq.length, 0);
            const fullClientMsg = Buffer.concat([DefaultFullClientWsHeader, payloadSizeBuffer, compressedReq]);
            this.ws.send(fullClientMsg);

            clearInterval(this.sendTimer);
            this.sendTimer = setInterval(() => { 
                if (this.audioBuffers.length) {
                    const sends = this.audioBuffers.splice(0, this.audioBuffers.length);
                    this.sendChunk(Buffer.concat(sends))
                }
            }, 200)
        });
        this.ws.on('message', (data) => {
            this.onMessage && this.onMessage(this.parseResponse(data));
        });
        this.ws.on('error', (data) => {
            console.log(data);
            this.onError && this.onError(data);
        });
        this.ws.on('close', (data) => {
            this.onClose && this.onClose(data);
        });
    }

    // 使用 zlib 进行 gzip 压缩
    gzipCompress(inputBuffer) {
        return zlib.gzipSync(inputBuffer);
    }

    // 使用 zlib 进行 gzip 解压
    gzipDecompress(inputBuffer) {
        return zlib.gunzipSync(inputBuffer);
    }

    // 构造 full client 请求（JSON 对象转 Buffer）
    constructRequest() {
        const reqid = uuidv4();
        const req = {
            app: {
                appid: this.appid,
                cluster: this.cluster,
                token: this.token,
                rate: this.rate
            },
            user: {
                uid: this.uid

            },
            request: {
                reqid: reqid,
                nbest: 1,
                workflow: this.workflow,
                result_type: "full",
                sequence: 1
            },
            audio: {
                format: this.format,
                codec: this.codec,
                channel: this.channel
            }
        };
        const reqStr = JSON.stringify(req);
        return Buffer.from(reqStr);
    }

    // 解析服务器返回的二进制消息
    parseResponse(msgBuffer) {
        const header0 = msgBuffer[0];
        const headerSize = header0 & 0x0f;  // header 的字节数除以 4
        const headerBytes = headerSize * 4;
        const messageType = msgBuffer[1] >> 4;
        const serializationMethod = msgBuffer[2] >> 4;
        const messageCompression = msgBuffer[2] & 0x0f;

        const payload = msgBuffer.slice(headerBytes);
        let payloadMsg;
        let payloadSize = 0;

        if (messageType === SERVER_FULL_RESPONSE) {
            payloadSize = payload.readUInt32BE(0);
            payloadMsg = payload.slice(4);
        } else if (messageType === SERVER_ACK) {
            const seq = payload.readUInt32BE(0);
            if (payload.length >= 8) {
                payloadSize = payload.readUInt32BE(4);
                payloadMsg = payload.slice(8);
            }
            console.log("SERVER_ACK seq:", seq);
        } else if (messageType === SERVER_ERROR_RESPONSE) {
            const code = payload.readUInt32BE(0);
            payloadSize = payload.readUInt32BE(4);
            payloadMsg = payload.slice(8);
            console.error("SERVER_ERROR_RESPONSE code:", code);
            // throw new Error(payloadMsg.toString());
        }

        if (payloadSize === 0) {
            // throw new Error("payload size is 0");
        }

        // 如果消息经过 gzip 压缩，则先解压
        if (messageCompression === GZIP) {
            payloadMsg = this.gzipDecompress(payloadMsg);
        }

        let asrResponse = {};
        if (serializationMethod === JSON_TYPE) {
            asrResponse = JSON.parse(payloadMsg.toString());
        }

        return asrResponse;
    }

    sendChunk(chunk, isLastSegment) {
        // test... 
        // this.writeStreamMP3.write(chunk);
        let audioMsgHeader;
        if (!isLastSegment) {
            audioMsgHeader = DefaultAudioOnlyWsHeader;
        } else {
            audioMsgHeader = DefaultLastAudioWsHeader;
        }
        const compressedAudio = this.gzipCompress(chunk);
        const audioPayloadSizeBuffer = Buffer.alloc(4);
        audioPayloadSizeBuffer.writeUInt32BE(compressedAudio.length, 0);
        const audioMsg = Buffer.concat([audioMsgHeader, audioPayloadSizeBuffer, compressedAudio]);
        this.ws.send(audioMsg);
    }

    send(audioData) {
        this.audioBuffers.push(audioData);
    }
    // 等待 WebSocket 返回下一条消息
    waitForMessage(ws) {
        return new Promise((resolve, reject) => {
            this.ws.once('message', (data) => {
                resolve(data);
            });
            this.ws.once('error', (err) => {
                reject(err);
            });
        });
    }

    close() {
        clearInterval(this.sendTimer);
        this.ws.close();
    }
    async end() {
        clearInterval(this.sendTimer);
        this.audioBuffers.length && this.sendChunk(Buffer.concat(this.audioBuffers), true)
    }
}


/**
 * 火山语音识别
 * @param {String}      device_id           设备ID
 * @param {String}      session_id          会话ID
 * @param {Number}      devLog              日志输出等级，为0时不应该输出任何日志
 * @param {Object}      iat_config          用户配置的 apikey 等信息
 * @param {String}      iat_server          用户配置的 iat 服务
 * @param {String}      llm_server          用户配置的 llm 服务
 * @param {String}      tts_server          用户配置的 tts 服务
 * @param {Function}    logWSServer         将 ws 服务回传给框架，如果不是ws服务可以这么写: logWSServer({ close: ()=> {} })
 * @param {Function}    iatServerErrorCb    与 TTS 服务之间发生错误时调用，并且传入错误说明，eg: ttsServerErrorCb("意外错误")
 * @param {Function}    cb                  IAT 识别的结果调用这个方法回传给框架 eg: cb({ text: "我是语音识别结果"  })
 * @param {Function}    logSendAudio        记录发送音频数据给服务的函数，框架在合适的情况下会进行调用
 * @param {Function}    connectServerBeforeCb 连接 iat 服务逻辑开始前需要调用这个方法告诉框架：eg: connectServerBeforeCb()
 * @param {Function}    connectServerCb     连接 iat 服务后需要调用这个方法告诉框架：eg: connectServerCb(true)
 * @param {Function}    serverTimeOutCb     当 IAT 服务连接成功了，但是长时间不响应时
 * @param {Function}    iatEndQueueCb       iat 静默时间达到后触发， 一般在这里面进行最后一帧的发送，告诉服务端结束识别
 * @param {Function}    log                 为保证日志输出的一致性，请使用 log 对象进行日志输出，eg: log.error("错误信息")、log.info("普通信息")、log.iat_info("iat 专属信息")
 * @param {(pluginsName: String)=> ({ sample_rate?: Number; channels?: Number; format?: String; language?: String })}  iat 静默时间达到后触发， 一般在这里面进行最后一帧的发送，告诉服务端结束识别 
 *
 *
*/
function IAT_FN({ device_id, session_id, log, devLog, iat_config, cb, logWSServer, logSendAudio, onIATText, connectServerCb, connectServerBeforeCb, iatEndQueueCb, getClientAudioConfig = ()=> ({}) }) {
    try {
        const { url = "", appid, accessToken, vad_course = 5000, clusterId = "volcengine_streaming_common" } = iat_config;
        if (!appid) return log.error(`请配给 IAT 配置 appid 参数。`)
        if (!accessToken) return log.error(`请配给 IAT 配置 accessToken 参数。`)
        if (!clusterId) return log.error(`请配给 IAT 配置 clusterId 参数。`)


        // 如果关闭后 message 还没有被关闭，需要定义一个标志控制
        let shouldClose = false;
        let iat_server_connected = false;
        let ended = false;
        let astText = "";

        let asrTimeoutTimer = null;
        let prevIsNull = false;

        connectServerBeforeCb();

        const client = new volcengineAsrClient({ url, appid, token: accessToken, cluster: clusterId, uid: device_id, getClientAudioConfig });
        client.onOpen = () => {
            if (shouldClose) return;
            iat_server_connected = true;
            connectServerCb(true); 
        };
        client.onMessage = (data) => {
            if (shouldClose) return;
            astText = data?.result?.[0]?.text || "";
            onIATText && onIATText(astText);
            devLog === 2 && log.iat_info('识别内容:' + astText);
            if (iat_server_connected === false && !ended) {
                ended = true;
                devLog && log.iat_info('ASR 识别结果:' + astText);
                cb({ text: astText || "", device_id });
                client.close()
            }

            // 如果长时间无法识别文字需要关闭连接
            !prevIsNull && clearTimeout(asrTimeoutTimer);
            if (!astText) {
                prevIsNull = true;
                if (!prevIsNull) {
                    asrTimeoutTimer = setTimeout(() => {
                        ended = true;
                        devLog && log.iat_info('ASR 识别结果:' + astText);
                        cb({ text: astText || "", device_id });
                        client.close()
                    }, vad_course)
                }
            } else {
                prevIsNull = false;
            }

        };
        client.onClose = () => {
            devLog && log.iat_info("-> 火山 ASR 服务已关闭：", session_id)
            shouldClose = true;
            iat_server_connected = false;
            connectServerCb(false);
        };

        logWSServer({
            close: () => {
                iat_server_connected = false;
                shouldClose = true;
                devLog && log.iat_info('框架调用 ASR 关闭:' + session_id);
                client.close()
                connectServerCb(false);
            },
            end: async () => {
                devLog && log.iat_info('ASR 服务结束:' + session_id);
                if (iat_server_connected) {
                    iat_server_connected = false;
                    client.end();
                }
            }
        });

        // 当达到静默时间后会自动执行这个任务
        iatEndQueueCb(() => {
            if (shouldClose) return;
            if (iat_server_connected) {
                client.end();
                client.close()
            }
        })
        logSendAudio((data) => {
            // console.log("发送数据：", data);
            // console.log(shouldClose, iat_server_connected);
            if (shouldClose) return;
            if (!iat_server_connected) return;
            if (!data) return;
            client.send(data);
        })

    } catch (err) {
        connectServerCb(false);
        console.log(err);
        log.error("火山 ASR 插件错误：", err)
    }
}

module.exports = IAT_FN;
