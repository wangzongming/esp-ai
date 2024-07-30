const espAi = require("esp-ai"); 

// 详情配置项见： https://xiaomingio.top/esp-ai/server.html
const config = {
    port: 8080,
    // llm_server: "dashscope",
    api_key: {
        // 讯飞：https://console.xfyun.cn/services/iat  。打开网址后，右上角三个字段复制进来即可。
        xun_fei: {
            appid: "5200d300",
            apiSecret: "xxx",
            apiKey: "xx",
            llm: "v4.0",
        },
        // 阿里积灵（千问等）： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope: {
            apiKey: "sk-xx",
            // LLM 版本
            llm: "qwen-turbo",
        },


        // 火山引擎（豆包等）：https://console.volcengine.com/speech/service/8?AppID=6359932705
        volcengine: {
            // 火山引擎的TTS与LLM使用不同的key，所以需要分别配置
            tts: {
                // 服务接口认证信息
                appid: "xxx",
                accessToken: "xxx",
            },

            // 暂不支持 llm
            llm: {
                // 获取地址：https://console.volcengine.com/ark/region:ark+cn-beijing/endpoint?current=1&pageSize=10
                model: "ep-xxx",// 每个模型都有一个id
                // 获取地址：https://console.volcengine.com/ark/region:ark+cn-beijing/apiKey
                apiKey: "32dacfe4xxx",
            }
        }, 
    },
    /**
     * 客户端鉴权, 客户端首次连接与每一次调用接口都会进行回调。
     * 返回 success: false, 如 Promise.resolve({ success: false, message:"ak无效" }) 可使客户端鉴权失败
     * 返回 success: true,  如 Promise.resolve({ success: true }) 可使客户端鉴权成功
     * @param {object} params 参数为客户端中配置的参数， 这里会解析为字面量对象，开发者直接使用 key 方式引用即可。
     * @param {string} scene 什么场景下的鉴权, "connect" 连接时， "start_session" 开始会话时
     */
    auth: async (params, scene) => {
        // console.log(scene, params);
        return { success: true }
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
        // params.voice_type = "BV051_streaming"
        // params.voice_type = "BV021_streaming"  

        // 改完后一定要返回出去
        return params;
    },
};

espAi(config);