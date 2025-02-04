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


interface MusicFnResponse {
    // 音频地址，不返回时会提示 message 信息
    url?: string;
    // 音频开始时间 可以是数字（以秒为单位）或时间戳字符串（格式为 [[hh：]mm：]ss[.xxx]）。
    seek?: number;
    // 找不到音频地址时提示用户的信息，默认为： 没有找到相关的结果，换个关键词试试吧！
    message?: string;
}

type LLM_historysType = {
    "role": "user" | "assistant" | "system",
    "content": string
}[]

interface IntentionType {
    // 关键词
    key: string[] | ((text: string, args: {
        // LLM 对话历史
        llm_historys: LLM_historysType;
        // 上个对话是否正在播放音频， 如果要判断上一曲，下一曲，那就需要用到了
        prev_play_audio_ing: boolean;
    }) => Promise<MusicFnResponse | boolean>),
    /**
     * 向客户端发送的指令字符串，
     * 当为函数时会直接执行而不是向客户端发送字符串，当函数返回字符串时设备会直接播放文字，也可以不返回
    */
    instruct: "__sleep__" | "__play_music__" | "__io_high__" | "__io_low__" | "__send_pwm__" | string | ((args: {
        device_id: string;
        instance: Instance;
        text: string;
    }) => void | string),
    // 客户端执行指令[后]的回复消息（如：打开电灯完毕/关闭电灯完毕）
    message?: string;
    // 附加参数, 不管什么数据，都需要写为 string 类型，且不建议放较大的数据在这里
    data?: string;
    // io 或者 pwm 时需要配置的引脚
    pin?: number;
    // 超体 api_key, 该超体下如果没有指定的设备ID将会报错
    // 配置 api_key 后字符串类型的指令会进行NLP推理。
    api_key?: string;
    /***
     * nlp 服务地址，默认为 https://espai.natapp4.cc/v1/semantic。 注意，必须配置 api_key 才会去请求这个服务
     * 请求体为 json： {
     *      
     *   "api_key": api_key,
     *   "texts": ["开灯", "帮我开灯"]
     * }
     * 
     * 接口需要返回： true || false
     * 
    */
    nlp_server?: string;
    // 远程设备
    target_device_id?: string;
    /***
     * 音乐指令 __play_music__ 专用
     * 音乐播放服务
     * 用于返回音乐地址的服务，`esp-ai` 目前不提供音乐服务
    */
    music_server?: (name?: string, arg?: any) => Promise<MusicFnResponse>;
    /**
     * 音乐指令 __play_music__ 专用
    * 当音频结束后的回调
    * @param {object} arg.break_second  停止时的进度，单位秒。也就是用户播放了到了多少秒（seek+play_time）
    * @param {object} arg.play_time     实际播放音频的时间，单位秒。
    * @param {object} arg.seek          音频开始播放时间，其实也就是 music_server 函数中返回的 seek 值
    * @param {object} arg.start_time    开始播放音频的 Unix 毫秒数时间戳
    * @param {object} arg.end_time      结束播放音频的 Unix 毫秒数时间戳
    * @param {object} arg.event         结束原因： "user_break" 用户打断 | play_end 播放完毕 | ws_disconnect 设备断开  | foo 未知事件
    * @return {MusicFnResponse}         返回音频信息时会继续播放，如果返回空则结束播放
   */
    on_end: (arg: {
        break_second: number,
        play_time: number,
        seek: number,
        start_time: number,
        end_time: number,
        event: "user_break" | "play_end" | "ws_disconnect" | "foo"
    }) => Promise<MusicFnResponse | void>;

}

export interface Config {
    /**
     * 服务端口, 默认 8088
    */
    port?: number,

    /**
     * 日志输出模式：0 不输出(线上模式)， 1 普通输出， 2 详细输出
    */
    devLog?: number,

    /**
     * 语音识别开始前"嘟"的音频流，默认为 false，也就是不开启提示音
     * 只能播放本地 mp3 地址： iatDu: path.join(__dirname, `./du.mp3`) // nodejs 写法
     * 为 false 时关闭提示音，为 true 时使用默认提示音。
    */
    iatDu?: string | boolean;


    /**
     * 缓存 TTS 数量，根据自己的服务器能力来设置。 设置为 0 时，不开启 TTS 缓存
     * 默认 1000
    */
    cache_TTS_number?: boolean;


    /**
     * llm 对话历史保留多少回合，一问一答为一回合，默认 5 回合，也就是 10 句问答
    */
    llm_qa_number?: number;

    /**
     * 可以根据业务需求用这个方法去库中请求配置等
     * 客户端配置生成，主要是生成 IAT/LLM/TTS 配置。客户端首次连接时会执行或者在某个空闲时刻内部会有自动更新策略
     * @param {string} params.send_error_to_client  向客户端发送自定义错误信息，客户端使用 onError 回调可以监听到。eg: send_data(500, "服务错误")
     * @param {string} params.ws  ws 对象，不建议使用
     * @param {string} params.client_params  配网页面配置的客户端参数
     *
     * 返回 success: false 客户端 onError 可监听到错误, 如 Promise.resolve({ success: false, message:"ak无效" }) 可使客户端鉴权失败. 5s 后服务端将会自动释放资源与断开连接
     *
     */
    gen_client_config: (params: Record<string, any>) => Promise<{ success: false, message: string } | {
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
         * 被唤醒后的回复，设置为空时使用默认的 "您好"，设置为 false 时不播放问候语
        */
        f_reply?: string | boolean;

        /**
         * 要退下时的回复
        */
        sleep_reply?: string;

        /**
         * 意图表：当用户唤醒 小明同学 后，小明同学可以做下面的任务
        */
        intention?: IntentionType[];

        /**
         * 初始化 LLM 时的提示语
        */
        llm_init_messages: Record<string, string>[]

    }>;


    /**
     * 客户端鉴权, 客户端首次连接与每一次调用接口都会进行回调。
     * 返回 success: false 客户端 onError 可监听到错误, 如 Promise.resolve({ success: false, message:"ak无效" }) 可使客户端鉴权失败. 5s 后服务端将会自动释放资源与断开连接
     * 返回 success: true,  如 Promise.resolve({ success: true }) 可使客户端鉴权成功
     * @param {string} params.type 什么场景下的鉴权, "connect" 连接时， "start_session" 开始会话时
     * @param {string} params.send_error_to_client  向客户端发送自定义错误信息，客户端使用 onError 回调可以监听到。eg: send_data(500, "服务错误")
     * @param {string} params.ws  ws 对象，不建议使用
     * @param {string} params.client_params  配网页面配置的客户端参数
     */
    auth?: (params: {
        type: "connect" | "start_session",
        send_error_to_client: (code: number, message: string) => void,
        ws: WebSocket;
        client_params: {
            api_key: string;
            ext1: string;
            ext2: string;
        }
    }) => Promise<{ success: boolean, message?: string }>;

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
     * @param {string} client_params  配网页面配置的客户端参数
     * @param {string} instance       ESP-AI 实例
     */
    onDeviceConnect?: (arg: { device_id: string, client_version: string; client_params: Record<string, any>, instance: Instance }) => void;

    /**
     * 设备断开连接的回调
     * @param {string} device_id 设备id 
     * @param {string} client_params  配网页面配置的客户端参数
     * @param {string} instance       ESP-AI 实例
     */
    onDeviceDisConnect?: (arg: { device_id: string, instance: Instance, client_params: Record<string, any>, }) => void;

    /**
     * 设备休息时的回调
     * @param {string} device_id 设备id 
     * @param {string} client_params  配网页面配置的客户端参数
     * @param {string} instance       ESP-AI 实例
     */
    onSleep?: (arg: { device_id: string, instance: Instance, client_params: Record<string, any> }) => void;


    /**
     * 用户发出 iat 服务回调请求之前的回调
     * @param {string} device_id      设备id
     * @param {string} instance       ESP-AI 实例
    */
    onIAT?: (arg: { device_id: string, ws: WebSocket, instance: Instance }) => void;

    /**
     * iat 回调: 语音识别过程中的回调
     * @param {string}    device_id     设备id
     * @param {string}    text          语音转的文字
     * @param {()=>void}  sendToClient  调用这个方法后可以直接将文字发送到客户端，客户端使用 onEvent 接收、
     *
     * *****  调用 sendToClient() 后，客户端代码向下面这样写即可接收到音频流 ****
     * void on_command(String command_id, String data) {
     *      if (command_id === "on_iat_cb") {
     *          // some code...
     *      }
     * }
     * void setup() {
     *      ...
     *      esp_ai.onEvent(on_command);
     * }
    */
    onIATcb?: (arg: { device_id: string, text: string, ws: WebSocket, instance: Instance, sendToClient: () => void }) => void;

    /**
    * iat 回调: 语音识别完毕的回调，可以在这里面发出最后一帧到语音识别服务器等操作，
    * 推荐使用 onIATcb 来代替, 这个属性只有在特殊情况会调用一下
    * @param {string} device_id 设备id
    * @param {string} text 语音转的文字
   */
    onIATEndcb?: (arg: { device_id: string, text: string, ws: WebSocket, instance: Instance }) => void;



    /**
     * 每调用一次TTS服务就会执行的回调函数，也就是进行TTS转换前。注意转换前只能拿到待转换的文字
     * @param {string}   device_id       设备id
     * @param {Boolean}  is_over         是否完毕
     * @param {Buffer}   text            待转换文字
     * @param {()=>void} sendToClient    调用这个方法后可以直接将音频流发送到客户端，客户端使用 onEvent 接收，无论客户端要用音频流做什么都可以。
     *
     * *****  调用 sendToClient() 后，客户端代码向下面这样写即可接收文字 ****
     * void on_command(String command_id, String data) {
     *      if (command_id === "on_tts") {
     *          // some code...
     *      }
     * }
     * void setup() {
     *      ...
     *      esp_ai.onEvent(on_command);
     * }
    */
    onTTS?: (arg: { device_id: string, tts_task_id: string, text: string, ws: WebSocket, sendToClient: () => void, instance: Instance }) => void;

    /**
     * TTS 转换完毕后的回调，注意：onTTScb是TTS转换后的回调，可以拿到音频流。onTTS是转换前的回调，只能拿到文字。
     * @param {string}   device_id       设备id
     * @param {Boolean}  is_over         是否完毕
     * @param {Buffer}   audio           音频流, mp3 格式, 使用 base64 格式进行封装。自行解码为二进制即可。
     * @param {()=>void} sendToClient    调用这个方法后可以直接将音频流发送到客户端，客户端使用 onEvent 接收，无论客户端要用音频流做什么都可以。
     *
     * *****  调用 sendToClient() 后，客户端代码向下面这样写即可接收到音频流 ****
     * void on_command(String command_id, String data) {
     *      if (command_id === "on_tts_cb") {
     *          // some code...
     *      }
     * }
     * void setup() {
     *      ...
     *      esp_ai.onEvent(on_command);
     * }
     *
    */
    onTTScb?: (arg: { device_id: string, is_over: boolean, audio: Buffer, ws: WebSocket, sendToClient: () => void, instance: Instance }) => void;

    /**
     * llm 服务调用前的回调
     * @param {string}    device_id     设备id
     * @param {string}    text          输入的文本，也就是 asr 识别结果
     * @param {object[]}  llm_historys  对话历史
     * @param {()=>void}  sendToClient  调用这个方法后可以直接将文字发送到客户端，客户端使用 onEvent 接收、 。如果调用时传入了一段文本，那会把文本发给客户端，而不是发送 llm 的推理结果
     *
     * *****  调用 sendToClient() 后，客户端代码向下面这样写即可接收到音频流 ****
     * void on_command(String command_id, String data) {
     *      if (command_id === "on_cb") {
     *          // some code...
     *      }
     * }
     * void setup() {
     *      ...
     *      esp_ai.onEvent(on_command);
     * }
     *
    */
    onLLM?: (arg: { device_id: string, text: string, ws: WebSocket, sendToClient: (text: String) => void, instance: Instance }) => void;

    /**
     * LLM 推理后的回调，拿到的文字是推理结果。
     * @param {string}    device_id     设备id
     * @param {string}    text          大语言模型推理出来的文本片段
     * @param {string}    user_text     用户问题 
     * @param {string}    llm_text      大模型推理出来的完整文本  
     * @param {boolean}   is_over       是否回答完毕
     * @param {object[]}  llm_historys  对话历史
     * @param {()=>void}  sendToClient  调用这个方法后可以直接将文字发送到客户端，客户端使用 onEvent 接收。
     *
     * *****  调用 sendToClient() 后，客户端代码向下面这样写即可接收到音频流 ****
     * void on_command(String command_id, String data) {
     *      if (command_id === "on_llm_cb") {
     *          // some code...
     *      }
     * }
     * void setup() {
     *      ...
     *      esp_ai.onEvent(on_command);
     * }
     *
    */
    onLLMcb?: (arg: { device_id: string, user_text: string, text: string, llm_text: string, is_over: boolean, llm_historys: Record<string, any>[], ws: WebSocket, sendToClient: () => void, instance: Instance }) => void;

    /**
     * 插件
    */
    plugins?: {
        name: string;
        type: "LLM" | "TTS" | "IAT";
        main: (arg: Record<string, any>) => void;
    }[];

    /**
     * 自定义日志输出逻辑
     * 例如你可以将日志存文件中, 推荐配合 log4js 等插件使用
    */
    logs?: {
        // 普通消息
        info?: () => void;
        // 错误消息
        error?: () => void;
    }
}

type PinModeType = "OUTPUT" | "INPUT" | "INPUT_PULLUP" | "INPUT_PULLDOWN";
type VoltageType = "LOW" | "HIGH";

export interface Instance {
    /**
     * 获取连接了的所有设备, 或者指定设备ID的设置
    */
    getClients(device_id?: string): Record<string, any>;

    /**
     * 更新客户端配置也就是 gen_client_config 配置返回出来的数据
    */
    updateClientConfig(device_id: string, config: Record<string, any>): void;

    /**
      * 服务端设置客户端端wifi信息的方法
      * 设置客户端 wifi 信息和存贮的业务数据，也就是配网页面设置的值，都可以用这个方法来改
      * 等同于硬件端的 .setWifiConfig 方法
      * wifi_name | wifi_pwd | api_key | ext1 | ext2 | ext3 | ext4 | ext5
     */
    setWifiConfig(device_id: string, arg: Record<"wifi_name" | "wifi_pwd" | "api_key" | "ext1" | "ext2" | "ext3" | "ext4" | "ext5" | "ext6" | "ext7", string>): Promise<boolean>;

    /**
     * 让客户端输出一段话
    */
    tts(device_id: string, text: string, opts: Record<string, any>): Promise<boolean>;

    /**
     * 终止会话，包括：语音识别、TTS、LLM
     *
     * @param at 主要用户日志输出，在什么xxx时候断开的会话
    */
    stop(device_id: string, at?: string): Promise<boolean>;

    /***
     * 终止会话
     * 一般配合 .stop 使用，用于重启一个会话
     * 返回 session_id
    */
    newSession(device_id: string): Promise<string>;

    /**
    * 匹配某个命令，如果匹配上会执行
    *
    *@param reply 回复语，如果是用户手动按按钮的情况下，一般不使用 message，而是使用自定义的提示语
   */
    matchIntention(device_id: string, text: string, reply: string): Promise<IntentionType>;


    /**
     * 重启设备
    */
    restart(device_id: string): Promise<void>;


    /**
     *  手动设置设备本地存储的数据，值为空字符串时为清空
     *  和 setWifiConfig 的区别是：本函数可将值设置为空字符串， setWifiConfig 为批量更新，空字符串会直接省略
    */
    setLocalData(device_id: string, field: string, value: string): Promise<void>

    /**
     * 设置用户的上下文，当对话存在多角色时需要在业务代码中自行调用本方法进行切换会话
     * @param llm_historys {"role": "user" | "assistant" | "system", "content":string}[]
    */
    setLLMHistorys(device_id: string, llm_historys: LLM_historysType): void;

    /**
     * 获取用户的上下文，在设置上下文时一般需要将当前上下文先存起来，否则切换回来时会丢失
     * @return llm_historys {"role": "user" | "assistant" | "system", "content":string}[]
    */
    getLLMHistorys(device_id: string): LLM_historysType;

    /**
     * 获取设备是否正在播放音频, 注意：不是TTS, 而是 __play_music__ 指令触发的音频
    */
    isPlaying(device_id: string): boolean;

    /**
     * 设置引脚引脚模式。
     * 功能和 Arduino 的 pinMode 一样
    */
    pinMode(device_id: string, pin: number, type: PinModeType): boolean;

    /**
     * 设置引脚电平
     * 功能和 Arduino 的 digitalWrite 一样，使用前必须使用 pinMode 将引脚设置为 OUTPUT 模式。
     * 将引脚电平设置为高电平或者低电平， 输出电压以开发板为准，如 esp32s3 开发板输出 3.3v 电压
     * 使用场景：控制继电器闭合、点亮led等等
    */
    digitalWrite(device_id: string, pin: number, type: VoltageType): boolean;

    /**
     * 读取引脚电平
     * 功能和 Arduino 的 digitalRead 一样，使用前必须使用 pinMode 将引脚设置为 INPUT 模式。 注意：本方法存在 100ms 的延时
     * 使用场景：读取按钮是否按下等等
    */
    digitalRead(device_id: string, pin: number, onChange: (val: VoltageType) => void): void;

    /**
     * 引脚模拟输出
     * 功能和 Arduino 的 analogWrite 一样，使用前必须使用 pinMode 将引脚设置为 OUTPUT 模式。
     * 使用场景：使用 PWM 控制电机转速、舵机角度 等等
    */
    analogWrite(device_id: string, pin: number, val: number): boolean;

    /**
     * 读取引脚模拟输入
     * 功能和 Arduino 的 analogRead 一样。注意：本方法存在 100ms 的延时
     * 使用场景： 读取电位器的值等等
    */
    analogRead(device_id: string, pin: number, onChange: (val: number) => void): void;
}
