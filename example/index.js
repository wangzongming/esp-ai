// const espAi = require("esp-ai");
const espAi = require("../../server");

espAi({
    llm_server: "dashscope",
    api_key: {
        // 讯飞：https://console.xfyun.cn/services/iat  。打开网址后，右上角三个字段复制进来即可。
        xun_fei: {
            appid: "5200d300",
            apiSecret: "ZjQwYzQ4YTUzN2UxY2RkNjRkMzAwMGVm",
            apiKey: "9cd6507384678076d9f5640573374fb7",
            llm: "v4.0",
        },
        // 阿里积灵（千问等）： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope: {
            apiKey: "sk-2a9afd13184c4b239739f25b370f8d21",
            // LLM 版本
            llm: "qwen-turbo",
        },


        // 火山引擎（豆包等）：https://console.volcengine.com/speech/service/8?AppID=6359932705
        volcengine: {
            // 火山引擎的TTS与LLM使用不同的key，所以需要分别配置
            tts: {
                // 服务接口认证信息
                appid: "6359932705",
                accessToken: "252Do7AY5Yz0CayLMXzyNr0BJNnygbU5",
            },

            // 暂不支持 llm
            llm: {
                // 获取地址：https://console.volcengine.com/ark/region:ark+cn-beijing/endpoint?current=1&pageSize=10
                model: "ep-xxx",// 每个模型都有一个id
                // 获取地址：https://console.volcengine.com/ark/region:ark+cn-beijing/apiKey
                apiKey: "32dacfe4xxx",
            }
        },

        ttson: {
            token: "ht-fc2dca28765428531557b377f5e19b24"
        },
    },
    tts_params_set: (params) => {

        /** 阿里积灵 **/
        // 说话人列表见：https://console.xfyun.cn/services/tts  
        // params.model = : "sambert-zhimiao-emo-v1" 

        /** 讯飞 **/
        // 说话人列表见：https://help.aliyun.com/zh/dashscope/developer-reference/model-list-old-version?spm=a2c4g.11186623.0.0.5fbe490eBdtzX0
        // params.vcn = "aisbabyxu";

        /** 火山引擎 **/
        // 说话人列表见：https://www.volcengine.com/docs/6561/97465
        params.voice_type = "BV051_streaming"
        // params.voice_type = "BV021_streaming" 

        /** 海豚配音 **/
        // token注册：https://www.ttson.cn/ 
        // 说话人列表见：https://github.com/wangzongming/esp-ai/tree/master/src/functions/tts/ttson/角色列表.yaml
        // params.voice_id = 2115;

        // 改完后一定要返回出去
        return params;
    },
});