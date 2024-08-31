const espAi = require("esp-ai"); 


// 详情配置项见： https://espai.fun/server.html
const config = {
    gen_client_config: ()=>({
        iat_server: "xun_fei",
        // iat_server: "esp-ai-plugin-iat-example",
        iat_config: {
            // 讯飞：https://console.xfyun.cn/services/iat  。打开网址后，右上角三个字段复制进来即可。
            appid: "xxx",
            apiSecret: "xxx",
            apiKey: "xxx",
            // 静默时间，多少时间检测不到说话就算结束，单位 ms
            vad_eos: 1500,
        },

        llm_server: "xun_fei", 
        llm_config: { 
            appid: "xxx",
            apiSecret: "xxx",
            apiKey: "xxx",
            llm: "v4.0", 
        },

        tts_server: "xun_fei", 
        tts_config: { 
            appid: "xxx",
            apiSecret: "xxx",
            apiKey: "xxx", 
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
    }), 
};

espAi(config);