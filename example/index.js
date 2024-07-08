const espAi = require("esp-ai");
espAi({
    // llm_server: "dashscope",
    api_key: {
        // 讯飞：https://console.xfyun.cn/services/iat  。打开网址后，右上角三个字段复制进来即可。
        xun_fei: {
            appid: "xxx",
            apiSecret: "xxx",
            apiKey: "xxx",
            // LLM 版本
            llm: "v4.0",  // v4.0 v3.5 v3.1 v2.1 v1.1（Lite）
        },
        // 阿里云-积灵： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope: {
            apiKey: "sk-xxx",
            // LLM 版本
            llm: "qwen-turbo",
        }
    },
});