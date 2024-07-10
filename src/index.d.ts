
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
     * 语音识别服务、TTS服务、LLM 服务的提供方, 默认为 xun_fei
     * @value xun_fei           讯飞的服务
     * @value dashscope         阿里-积灵
     * @value bai_du            百度的服务（预计 2.0 版本支持）
     * @value privatization     自己的服务（预计 2.0 版本支持）
     * @value oneapi            OPENAI接口类型的服务（预计 2.0 版本支持）
    */
    iat_server?: "xun_fei" | "dashscope" | "bai_du" | "privatization"| "oneapi",
    tts_server?: "xun_fei" | "dashscope" | "bai_du" | "privatization"| "oneapi",
    llm_server?: "xun_fei" | "dashscope" | "bai_du" | "privatization"| "oneapi",

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
        },
        // 阿里云-积灵： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope?: {
            apiKey: string,
            // LLM 版本
            llm?: string,
        },
        oneapi?: {
            apiKey: string,
            // LLM 版本
            llm?: string,
        }
    },

    /**
     * 意图表：当用户唤醒 小明同学 后，小明同学可以做下面的任务
    */
    intention?: {
        // 关键词
        key: string[],
        // 向客户端发送的指令
        instruct: "__sleep__" | string,
        message: string,
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
     * llm 参数控制, 可以设置温度等
     * @param {object} params 默认的llm参数
     */
    llm_params_set?: (params: Record<string, any>) => Record<string, any>;

    /**
     * 新设备连接服务的回调 
     * @param {string} device_id 设备id
     * @param {WebSocket} ws 连接句柄，可使用 ws.send() 发送数据
     */
    onDeviceConnect?: (arg: { device_id: string, ws: WebSocket }) => void;

    /**
     * iat 回调 
     * @param {string} device_id 设备id
     * @param {string} text 语音转的文字 
    */
    onIATcb?: (arg: { device_id: string, text: string }) => void;
    /**
     * tts 回调 
     * @param {string} device_id 设备id
     * @param {Buffer} is_over  是否完毕
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
}

export type Math = (config: Config) => void;
