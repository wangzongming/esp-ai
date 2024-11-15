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

const log = require("../../utils/log");
const isOutTimeErr = require("../../utils/isOutTimeErr");
/**
 * 客户端连接成功
*/
async function fn({ device_id }) {
    try {
        const { devLog, gen_client_config } = G_config;
        const { ws, client_params, client_version, error_catch, tts_buffer_chunk_queue } = G_devices.get(device_id);
        const user_config = await gen_client_config({
            client_params,
            ws,
            send_error_to_client: (code, message) => {
                ws.send(JSON.stringify({
                    type: "error",
                    message: message,
                    code: code
                }));
            }
        }) || {};
        if (user_config?.success === false) {
            // 服务报错 
            ws && ws.send(JSON.stringify({
                type: "error",
                at: "auth",
                code: isOutTimeErr(user_config.message) ? "007" : (user_config.code || "006"),
                message: `获取服务配置失败：${user_config.message}`
            }));
            setTimeout(() => {
                ws.close();
                // G_devices.delete(device_id);
            }, 5000)
            return;
        }
        if (!user_config.iat_server || !user_config.iat_config) {
            error_catch("IAT", "100", `请配置 iat_server、iat_config 参数。`);
            return log.error(`请配置 iat_server、iat_config 参数。`);
        }
        if (!user_config.llm_server || !user_config.llm_config) {
            error_catch("LLM", "200", `请配置 llm_server、llm_config 参数。`);
            return log.error(`请配置 llm_server、llm_config 参数。`);
        }
        if (!user_config.tts_server || !user_config.tts_config) {
            error_catch("TTS", "300", `请配置 tts_server、tts_config 参数。`);
            return log.error(`请配置 tts_server、tts_config 参数。`)
        }
        devLog && log.info(`---------------------------------------------------`);
        devLog && log.t_info(`客户端连接成功：${device_id}`);
        devLog && log.t_info(`客户端版本号：v${client_version}`);
        devLog && log.t_info(`用户配置 iat_server：${user_config.iat_server}`);
        devLog && log.t_info(`用户配置 iat_config：${JSON.stringify(user_config.iat_config)}`);
        devLog && log.t_info(`用户配置 llm_server：${user_config.llm_server}`);
        devLog && log.t_info(`用户配置 llm_config：${JSON.stringify(user_config.llm_config)}`);
        devLog && log.t_info(`用户配置 tts_server：${user_config.tts_server}`);
        devLog && log.t_info(`用户配置 tts_config：${JSON.stringify(user_config.tts_config)}`);
        devLog && log.info(`---------------------------------------------------`);

        const _user_config = {
            f_reply: "您好",
            sleep_reply: "我先退下了，有需要再叫我。",
            connected_reply: "后台服务连接成功",
            ...user_config,
        }
        G_devices.set(device_id, {
            ...G_devices.get(device_id),
            user_config: _user_config,
        })

        const TTS_FN = require(`../tts`);
        const { user_config: { connected_reply, intention = [] } } = G_devices.get(device_id);

        ws && ws.send(JSON.stringify({ type: "stc_time", stc_time: +new Date() + "" }));

        intention.forEach(({ instruct, pin }) => {
            if (instruct === "__io_high__" || instruct === "__io_low__") {
                !pin && log.error(`__io_high__ 指令必须配置 pin 数据`);
                G_Instance.pinMode(device_id, pin, "OUTPUT");
            }
        })
 

        // 播放ws连接成功语音
        if (connected_reply) {
            TTS_FN(device_id, {
                text: connected_reply,
                reRecord: false,
                pauseInputAudio: true,
                onAudioOutOver: () => {
                    ws && ws.send("session_end");
                },
                text_is_over: true,
            })
        }




        // const IAT_FN = require(`../iat`); 
        // const LLM_FN = require(`../llm`);
        // const play_temp = require(`../../audio_temp/play_temp`);
        // ============= 提示音测试 =============
        // play_temp("du.mp3", ws);   
        // play_audio("http://m10.music.126.net/20240723180659/13eabc0c9291dab9a836120bf3f609ea/ymusic/5353/0f0f/0358/d99739615f8e5153d77042092f07fd77.mp3", ws)


        // ============= 指令发送测试 ============= 
        // ws.send(JSON.stringify({ type: "instruct", command_id: "open_test", data: "这是数据" }));


        // // ============= TTS 测试 =============  
        // return TTS_FN(device_id, {
        //     text: "上一曲",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第1句播放完毕的回调')
        //     }
        // });
        // tts_buffer_chunk_queue.push(() => {
        //     return TTS_FN(device_id, {
        //         text: "你好",
        //         reRecord: false,
        //         pauseInputAudio: true,
        //         onAudioOutOver: () => {
        //             console.log('第1句播放完毕的回调')
        //         }
        //     });
        // })

        // // await new Promise(resolve => setTimeout(resolve, 1000));

        // tts_buffer_chunk_queue.push(() => {
        //     return TTS_FN(device_id, {
        //         text: "小明在的",
        //         reRecord: false,
        //         pauseInputAudio: true,
        //         onAudioOutOver: () => {
        //             console.log('第2句播放完毕的回调')
        //         }
        //     });
        // })

        /**
         * 108个字
         * 
         * PCM         数据大小：818 kb  |  火山 610kb
         * MP3         数据大小：155 kb  |  火山 384kb
         * ogg_opus    数据大小：622 kb
         * speex8k     数据大小：98  kb
         * speex16k    数据大小：77  kb
         * opus ...
        */
        // tts_buffer_chunk_queue.push(() => {
        //     return TTS_FN(device_id, {
        //         text: "第三句，经向中国主管部门核实，中国驻泰国使馆再次澄清，为帮助下游地区应对洪灾，中方近来持续稳定和减少景洪水电站出库流量，不可能对下游地区抗洪救灾形成压力。中方愿继续与流域国家加强沟通合作，共同应对极端天气造成的影响。",
        //         reRecord: false,
        //         pauseInputAudio: true,
        //         onAudioOutOver: () => {
        //             console.log('第3句播放完毕的回调')
        //         }
        //     });
        // })



        // await TTS_FN(device_id, {
        //     text: "小明在的",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第一句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第二句，萌娃音色要上线啦！",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第二句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第三句！第三句！第三句！",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第三句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第四句，经向中国主管部门核实，中国驻泰国使馆再次澄清，为帮助下游地区应对洪灾，中方近来持续稳定和减少景洪水电站出库流量，不可能对下游地区抗洪救灾形成压力。中方愿继续与流域国家加强沟通合作，共同应对极端天气造成的影响。",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第4句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第五句，经向中国主管部门核实，中国驻泰国使馆再次澄清，为帮助下游地区应对洪灾，中方近来持续稳定和减少景洪水电站出库流量，不可能对下游地区抗洪救灾形成压力。中方愿继续与流域国家加强沟通合作，共同应对极端天气造成的影响。",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第5句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第六句，经向中国主管部门核实，中国驻泰国使馆再次澄清，为帮助下游地区应对洪灾，中方近来持续稳定和减少景洪水电站出库流量，不可能对下游地区抗洪救灾形成压力。中方愿继续与流域国家加强沟通合作，共同应对极端天气造成的影响。",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第6句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第七句，经向中国主管部门核实，中国驻泰国使馆再次澄清，为帮助下游地区应对洪灾，中方近来持续稳定和减少景洪水电站出库流量，不可能对下游地区抗洪救灾形成压力。中方愿继续与流域国家加强沟通合作，共同应对极端天气造成的影响。",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第7句播放完毕的回调')
        //     }
        // });
        // await TTS_FN(device_id, {
        //     text: "第八句，经向中国主管部门核实，中国驻泰国使馆再次澄清，为帮助下游地区应对洪灾，中方近来持续稳定和减少景洪水电站出库流量，不可能对下游地区抗洪救灾形成压力。中方愿继续与流域国家加强沟通合作，共同应对极端天气造成的影响。",
        //     reRecord: false,
        //     pauseInputAudio: true,
        //     onAudioOutOver: () => {
        //         console.log('第8句播放完毕的回调')
        //     }
        // }); 

    } catch (err) {
        console.log(err);
        log.error(`[${device_id}] play_audio_ws_conntceed 消息错误： ${err}`)
    }

}

module.exports = fn