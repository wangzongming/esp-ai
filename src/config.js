/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const config = {
    /**
     * 服务端口, 默认 8080
    */
    port: 8080,

    /**
     * 日志输出模式：0 不输出(线上模式)， 1 普通输出， 2 详细输出
    */
    devLog: 1,

    /**
     * 语音识别服务、TTS服务、LLM 服务的提供方, 默认为 xun_fei
     * @value xun_fei           讯飞的服务
     * @value dashscope         阿里-积灵
     * @value bai_du            百度的服务（预计 2.0 版本支持）
     * @value privatization     自己的服务（预计 2.0 版本支持）
    */
    iat_server: "xun_fei",

    tts_server: "xun_fei",

    llm_server: "xun_fei",
    // llm_server: "dashscope",

    /**
     * 不同的服务商需要配置对应的 key
     * 每个服务商配置不是完全一样的，具体请参考文档
    */
    api_key: {
        // 讯飞：https://console.xfyun.cn/services/iat  。打开网址后，右上角三个字段复制进来即可。
        xun_fei: {
            appid: "",
            apiSecret: "",
            apiKey: "",
            // LLM 版本
            llm: "v3.5",
        },
        // 阿里云-积灵： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope: {
            apiKey: "sk-",
            // LLM 版本
            llm: "qwen-turbo",
        }
    },

    /**
     * 意图表：当用户唤醒 小明同学 后，小明同学可以做下面的任务
    */
    intention: [
        {
            // 关键词
            key: ["帮我开灯", "开灯", "打开灯"],
            // 向客户端发送的指令
            instruct: "device_open_001",
            message: "开啦！还有什么需要帮助的吗？"
        },
        {
            // 关键词
            key: ["帮我关灯", "关灯", "关闭灯"],
            // 向客户端发送的指令
            instruct: "device_close_001",
            message: "关啦！还有什么需要帮助的吗？"
        },
        {
            // 关键词
            key: ["退下吧", "退下"],
            // 内置的睡眠指令
            instruct: "__sleep__",
            message: "我先退下了，有需要再叫我。"
        }
    ],

    /**
     * 初始化LLM时的提示
    */
    llm_init_messages: [
        {
            "role": "system",
            "content": "你是一个非常厉害的智能助手！你的名字叫小明同学。"
        },
        {
            "role": "system",
            "content": "你擅长用精简的句子来回答用户的问题。每次回答用户的问题最多都不会超过50字。除非用户指定多少字。"
        },
    ],

    /**
     * 被唤醒后的回复
    */
    f_reply: "小明在的",

    /**
     * 休息时的回复
    */
    sleep_reply: "我先退下了，有需要再叫我。",

    /**
     * llm 参数控制, 可以设置温度等
    */
    // llm_params_set: (params) => {
    //     // 千问top_p、top_k ...
    //     // params.top_p = 0.5;     

    //     // 讯飞 temperature ...
    //     // params.parameter.chat.temperature = 0.4;
    //     // params.parameter.chat.max_tokens = 100;

    //     // 改完后一定要返回出去
    //     return params;
    // },


    /**
     * 新设备连接服务的回调 
     * @param {string} device_id 设备id
     * @param {WebSocket} ws 连接句柄，可使用 ws.send() 发送数据
    */
    // onDeviceConnect({ device_id, ws }) { },

    /**
     * iat 回调 
     * @param {string} device_id 设备id
     * @param {string} text 语音转的文字 
    */
    // onIATcb({ device_id, text }) { },
    
    /**
     * tts 回调 
     * @param {string} device_id 设备id
     * @param {Buffer} is_over  是否完毕
     * @param {Buffer} audio    音频流
    */
    // onTTScb({ device_id, is_over, audio }) { },

    /**
     * llm 回调 
     * @param {string} device_id 设备id
     * @param {string} text 大语言模型推理出来的文本片段 
     * @param {boolean} is_over 是否回答完毕 
     * @param {object[]} llm_historys 对话历史 
     * 
    */
    // onLLMcb({ device_id, text, is_over, llm_historys }) { },
}

module.exports = config;
