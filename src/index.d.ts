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
export type Config = {
    /**
     * 服务端口, 默认 8080
    */
    port?: number,

    /**
     * 日志输出模式：0 不输出(线上模式)， 1 普通输出， 2 详细输出
    */
    devLog?: number,

    /**
     * 可以根据业务需求用这个方法去库中请求配置等...
     * 客户端配置生成，主要是生成 IAT/LLM/TTS 配置。客户端首次连接时会执行或者在某个空闲时刻内部会有自动更新策略 
     * @param {object} params 参数为客户端中配置的参数， 这里会解析为字面量对象，开发者直接使用 key 方式引用即可。 
     */
    gen_client_config: (params: Record<string, any>) => Promise<{
        /**
         * 语音识别服务、TTS服务、LLM 服务的提供方, 默认为 xun_fei
         * @value xun_fei           讯飞的服务
         * @value dashscope         阿里-积灵  
         * @value volcengine        火山引擎（豆包等）
         * @value [string]          自定义插件
        */
        iat_server?: "xun_fei" | "dashscope" | "volcengine" | string;
        iat_config: {
            [key: string]: any;
        };

        tts_server?: "xun_fei" | "dashscope" | "volcengine" | string;
        tts_config: {
            [key: string]: any;
        };

        llm_server?: "xun_fei" | "dashscope" | "volcengine" | string;
        llm_config: {
            [key: string]: any;
        };

        /**
         * 客户端连接服务后的回复
        */
        connected_reply?: string,

        /**
         * 被唤醒后的回复
        */
        f_reply?: string;

        /**
         * 要退下时的回复
        */
        sleep_reply?: string;

        /**
         * 意图表：当用户唤醒 小明同学 后，小明同学可以做下面的任务
        */
        intention?: {
            // 关键词
            key: string[] | ((text: string, llm_historys: Record<string, string>[]) => { url: string; seek: string | number }),
            // 向客户端发送的指令
            instruct: "__sleep__" | "__play_music__" | "__io_high__" | "__io_low__" | "__send_pwm__" | string,
            // 客户端执行指令后的回复消息（如：打开电灯完毕/关闭电灯完毕）
            message?: string,
            // 附加参数, 不管什么数据，都需要写为 string 类型，且不建议放较大的数据在这里
            data?: string;
            // io 或者 pwm 时需要配置的引脚
            io?: number;
            /***
             * 音乐播放服务
             * 用于返回音乐地址的服务，`esp-ai` 目前不提供音乐服务
             * 
            */
            music_server?: (name?: string, arg?: any) => {
                // 音频地址，不返回时会提示 message 信息
                url?: string;
                // 音频开始时间 可以是数字（以秒为单位）或时间戳字符串（格式为 [[hh：]mm：]ss[.xxx]）。
                seek?: number;
                // 找不到音频地址时提示用户的信息，默认为： 没有找到相关的结果，换个关键词试试吧！
                message?: string;
            }
        }[];

        /**
         * 初始化 LLM 时的提示语
        */
        llm_init_messages: Record<string, string>[]

    }>;


    /**
     * 客户端鉴权, 客户端首次连接与每一次调用接口都会进行回调。
     * 返回 success: false, 如 Promise.resolve({ success: false, message:"ak无效" }) 可使客户端鉴权失败
     * 返回 success: true,  如 Promise.resolve({ success: true }) 可使客户端鉴权成功
     * @param {object} params 参数为客户端中配置的参数， 这里会解析为字面量对象，开发者直接使用 key 方式引用即可。
     * @param {string} scene 什么场景下的鉴权, "connect" 连接时， "start_session" 开始会话时
     */
    auth?: (params: Record<string, any>, scene: "connect" | "start_session") => Promise<{ success: boolean, message?: string }>;

    /**
     * llm 参数控制, 可以设置温度等
     * @param {object} params 默认的llm参数
     */
    llm_params_set?: (params: Record<string, any>) => Record<string, any>;

    /**
     * tts 参数控制, 可以设置说话人、音量、语速等
     * @param {object} params 默认的tts参数
     */
    tts_params_set?: (params: Record<string, any>) => Record<string, any>;


    /**
     * 新设备连接服务的回调 
     * @param {string} device_id 设备id
     * @param {string} client_version 客户端版本
     * @param {WebSocket} ws 连接句柄，可使用 ws.send() 发送数据
     */
    onDeviceConnect?: (arg: { device_id: string, ws: WebSocket, client_version: string }) => void;

    /**
     * 用户发出 iat 服务回调请求之前的回调
    */
    onIAT?: (arg: { device_id: string, ws: WebSocket, }) => void;

    /**
     * iat 回调: 语音识别过程中的回调
     * @param {string} device_id 设备id
     * @param {string} text 语音转的文字 
    */
    onIATcb?: (arg: { device_id: string, text: string, ws: WebSocket }) => void;

    /**
    * iat 回调: 语音识别完毕的回调，可以在这里面发出最后一帧到语音识别服务器等操作
    * @param {string} device_id 设备id
    * @param {string} text 语音转的文字 
   */
    onIATEndcb?: (arg: { device_id: string, text: string, ws: WebSocket }) => void;



    /**
     * 每调用一次TTS服务就会执行的回调函数，也就是进行TTS转换前
    */
    onTTS?: (arg: { device_id: string, tts_task_id: string, text: string, ws: WebSocket }) => void;
    /**
     * tts 回调 
     * @param {string} device_id 设备id
     * @param {Boolean} is_over  是否完毕
     * @param {Buffer} audio    音频流
    */
    onTTScb?: (arg: { device_id: string, is_over: boolean, audio: Buffer, ws: WebSocket }) => void;

    /**
     * llm 服务调用前的回调 
     * @param {string} device_id 设备id
     * @param {string} text 大语言模型推理出来的文本片段  
     * @param {object[]} llm_historys 对话历史 
     * 
    */
    onLLM?: (arg: { device_id: string, text: string, ws: WebSocket }) => void;

    /**
     * llm 回调 
     * @param {string} device_id 设备id
     * @param {string} text 大语言模型推理出来的文本片段 
     * @param {boolean} is_over 是否回答完毕 
     * @param {object[]} llm_historys 对话历史 
     * 
    */
    onLLMcb?: (arg: { device_id: string, text: string, is_over: boolean, llm_historys: Record<string, any>[], ws: WebSocket }) => void;

    /**
     * 插件
    */
    plugins?: {
        name: string;
        type: "LLM" | "TTS" | "IAT";
        main: (arg: Record<string, any>) => void;
    }[]
}

export type Math = (config: Config) => void;
