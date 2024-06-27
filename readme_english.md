
# ESP-AI  [![npm](https://img.shields.io/npm/v/esp-ai.svg)](https://www.npmjs.com/package/esp-ai) [![npm](https://img.shields.io/npm/dm/esp-ai.svg?style=flat)](https://www.npmjs.com/package/esp-ai)



![logo.png](./logo.png)

<a href="./readme.md">简体中文</a>

> Provide a complete set of AI dialogue solutions for your development board, including but not limited to the IAT+LLM+TTS integration solution for the ESP32 series development board.


You only need to configure the key of IAT, LLM, TTS services provided by IFlytek, Baidu, Ali Jaling, local services and other platforms to the plug-in, you can run the service, without considering the interaction between the various services, and do not need to consider the interaction between the development board and the service, you only need to do your robot ~


The server-side code of this project is based on Nodejs, and the hardware code is based on Arduino IDE.


Open source is not easy, the top right corner of a 'Star' support a little bit of it ~

# communication group
QQ group: 854445223

# How to run this project?

**Get an intelligent assistant up and running in three simple steps.**

1. Install esp-ai
2. Introduce the plug-in into your project, configure the relevant 'key' and run it
3. Burn the provided client code to the 'ESP32s3' development board

# Server required environment

nodejs >= v14.0.0 

# install
Create a new directory, or perform the following installation in your project directory.

Install using npm
```
npm i esp-ai
```
or
```
yarn add esp-ai
```

# Pre-operation preparation
1. Register iFlytek Open Platform account
2. Obtain the appid and apikey of iFlytek Open Platform account

**llm can also register Alijiling to use**

# running service

In your project, alternatively add an index.js and write the following code.

```
const espAi = require("esp-ai");
 
espAi({ 
    api_key: {
        xun_fei: {
            appid: "xxx",
            apiSecret: "xxx",
            apiKey: "xxx",
            // LLM 版本
            llm: "v3.5",
        },
    },
});
```
Execute the following code to run the service
```
node ./index.js
```

Use pm2 to run services in the build environment to ensure service reliability and performance.

```
pm2 start ./index.js -i max
```

# Client
The client is written using the Arduino IDE, please install the Arduino IDE yourself first


1. Download /client/client.ino to the local PC
2. Open the file with Arduino IDE and upload it to the board (be careful to modify the relevant 'wifi' and pin configuration).
   

# Client wiring
## ESP32-s3
### INMP441(mic) 
| ESP32-s3 | INMP441 |
| -------- | ------- |
| 3v3      | VDD     |
| GND      | GND     |
| GND      | L/R     |
| 5        | WS      |
| 4        | SCK     |
| 6        | SD      |

### Max98357A(amplifier) 
| ESP32-s3 | Max98357A |
| -------- | --------- |
| 3v3      | VDD       |
| GND      | GND       |
| 17       | LRC       |
| 16       | BCLK      |
| 15       | DIN       |

### potentiometer(volume adjustment) （optional）
| ESP32-s3 | potentiometer |
| -------- | ------ |
| 3v3      | VDD    |
| GND      | GND    |
| OUT      | 34     |

### LED （optional）
| ESP32-s3 | LED  |
| -------- | ---- |
| GND      | GND  |
| 18       | + |

# Supported development boards

**✔️ Support**   **❗developing**   **❌ nonsupport**
 
| board        | IAT | LLM | TTS | 离线唤醒 |
| ------------- | --- | --- | --- | -------- |
| ESP32-s3      | ✔️   | ✔️   | ✔️   | ✔️        |
| nodemcu-32    | ✔️   | ✔️   | ✔️   | ❌        |
| ESP32-xiao-s3 | ❗   | ❗   | ❗   | ✔️        |
| ...     |


# Supported platforms
**✔️ Support**   **❗developing**   **❌ nonsupport**
 
| server           | IAT | LLM | TTS |
| ---------------- | --- | --- | --- |
| iFlytek              | ✔️   | ✔️   | ✔️   |
| 百度             | ❗   | ❗   | ❗   |
| Baidu         | ❗   | ❗   | ❗   |
| Alijline(qianwen...) | ✔️   | ✔️   | ✔️   |
| ...        |


# Off-line wake up scheme
**✔️ Support **   **❗developing**   **❌ nonsupport**
 
| project      | support  | repository                                 |
| ------------ | -------- | -------------------------------------------- |
| Edge Impulse | ✔️        | https://studio.edgeimpulse.com/studio/422422 |
| ESP-SR       | ❗        | ❗                                            |
| ...    |          |

# Implementation principle steps

INMP441(MIC) -> audio -> ESP32 -> wake-on-voice -> audio -> esp-ai -> IAT Server -> text -> esp-ai -> LLM -> reply -> esp-ai -> TTS -> esp-ai -> ESP32 -> max98357(loudspeaker)

待补充结构图

 
# configuration 
The 'Typescript' definition file is provided, and the 'TS' project can also be referred to normally.


```
const espAi = require("esp-ai");
espAi({
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
     * @value bai_du            百度的服务（预计 1.0 版本支持）
     * @value privatization     自己的服务（预计 1.0 版本支持）
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
            appid: "xxx",
            apiSecret: "ZjQwYxxxxxx",
            apiKey: "9cd65073xxxxx",
            // LLM 版本
            llm: "v3.5",
        },
        // 阿里云-积灵： https://dashscope.console.aliyun.com/apiKey
        // 积灵主要是提供llm（推荐使用这个llm服务）
        dashscope: {
            apiKey: "sk-xxxx",
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
    llm_params_set: (params) => {
        // 千问top_p、top_k ...
        // params.top_p = 0.5;     

        // 讯飞 temperature ...
        // params.parameter.chat.temperature = 0.4;
        // params.parameter.chat.max_tokens = 100;

        // 改完后一定要返回出去
        return params;
    },


    /**
     * 新设备连接服务的回调 
     * @param {string} device_id 设备id
     * @param {WebSocket} ws 连接句柄，可使用 ws.send() 发送数据
    */
    onDeviceConnect({ device_id, ws }) { },

    /**
     * iat 回调 
     * @param {string} device_id 设备id
     * @param {string} text 语音转的文字 
    */
    onIATcb({ device_id, text }) { },

    /**
     * tts 回调 
     * @param {string} device_id 设备id
     * @param {Buffer} is_over  是否完毕
     * @param {Buffer} audio    音频流
    */
    onTTScb({ device_id, is_over, audio }) { },

    /**
     * llm 回调 
     * @param {string} device_id 设备id
     * @param {string} text 大语言模型推理出来的文本片段 
     * @param {boolean} is_over 是否回答完毕 
     * @param {object[]} llm_historys 对话历史 
     * 
    */
    onLLMcb({ device_id, text, is_over, llm_historys }) { },
});
```

# other 
At present, due to the small number of training samples, the accuracy of voice wake-up is not high, and it will be continuously optimized.
The client code is not currently provided as a header file, and may change to a header file in the next release.
Welcome pr to build ~ together.
