
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
     * 语音识别静默时间, 单位毫秒，默认 2500
    */
    vad_eos?: number;

    /**
     * 语音识别服务、TTS服务、LLM 服务的提供方, 默认为 xun_fei
     * @value xun_fei           讯飞的服务
     * @value dashscope         阿里-积灵  
     * @value volcengine        火山引擎（豆包等）
     * @value [string]          自定义插件
    */
    iat_server?: "xun_fei" | "dashscope" | "volcengine" | string,
    tts_server?: "xun_fei" | "dashscope" | "volcengine" | string,
    llm_server?: "xun_fei" | "dashscope" | "volcengine" | string,

    /**
     * 不同的服务商需要配置对应的 key
     * 每个服务商配置不是完全一样的，具体请参考文档
    */
    api_key: {
        // 讯飞：https://console.xfyun.cn/services/iat  。打开网址后，右上角三个字段复制进来即可。
        xun_fei?: {
            appid: string,
            apiSecret: string,
            apiKey: string,
            // LLM 版本
            llm?: string,
        };
        // 阿里云-积灵： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope?: {
            apiKey: string,
            // LLM 版本
            llm?: string,
        };
        [nemae: string]: Record<string, any>
    },

    /**
     * 意图表：当用户唤醒 小明同学 后，小明同学可以做下面的任务
    */
    intention?: {
        // 关键词
        key: string[],
        // 向客户端发送的指令
        instruct: "__sleep__" | string,
        // 客户端执行指令后的回复消息（如：打开电灯完毕/关闭电灯完毕）
        message?: string,
        // 附加参数, 不管什么数据，都需要写为 string 类型，且不建议放较大的数据在这里
        data?: string;
    }[];

    /**
     * 被唤醒后的回复
    */
    f_reply?: string;

    /**
     * 休息时的回复
    */
    sleep_reply?: string;
  
    /**
     * 客户端鉴权, 客户端首次连接与每一次调用接口都会进行回调。
     * 返回 success: false, 如 Promise.resolve({ success: false, message:"ak无效" }) 可使客户端鉴权失败
     * 返回 success: true,  如 Promise.resolve({ success: true }) 可使客户端鉴权成功
     * @param {object} params 参数为客户端中配置的参数， 这里会解析为字面量对象，开发者直接使用 key 方式引用即可。
     * @param {string} scene 什么场景下的鉴权, "connect" 连接时， "start_session" 开始会话时
     */
    auth ?: (params: Record<string, any>, scene: "connect" | "start_session") => Promise<{ success: boolean, message?: string }>;

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
     * iat 回调: 语音识别过程中的回调
     * @param {string} device_id 设备id
     * @param {string} text 语音转的文字 
    */
    onIATcb?: (arg: { device_id: string, text: string }) => void;

    /**
    * iat 回调: 语音识别完毕的回调，可以在这里面发出最后一帧到语音识别服务器等操作
    * @param {string} device_id 设备id
    * @param {string} text 语音转的文字 
   */
    onIATEndcb?: (arg: { device_id: string, text: string }) => void;

    /**
     * tts 回调 
     * @param {string} device_id 设备id
     * @param {Boolean} is_over  是否完毕
     * @param {Buffer} audio    音频流
    */
    onTTScb?: (arg: { device_id: string, is_over: boolean, audio: Buffer }) => void;
    /**
     * llm 回调 
     * @param {string} device_id 设备id
     * @param {string} text 大语言模型推理出来的文本片段 
     * @param {boolean} is_over 是否回答完毕 
     * @param {object[]} llm_historys 对话历史 
     * 
    */
    onLLMcb?: (arg: { device_id: string, text: string, is_over: boolean, llm_historys: Record<string, any>[] }) => void;

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
