
# ESP-AI  [![npm](https://img.shields.io/npm/v/esp-ai.svg)](https://www.npmjs.com/package/esp-ai) [![npm](https://img.shields.io/npm/dm/esp-ai.svg?style=flat)](https://www.npmjs.com/package/esp-ai)


![logo.png](./logo.png)

<a href="./readme_english.md">English</a>

> 为你的开发板提供全套的AI对话方案，包括但不限于 `ESP32` 系列开发板的 `IAT+LLM+TTS` 集成方案。

你只需要将讯飞、百度、阿里积灵、本地服务等平台提供`IAT`、`LLM`、`TTS`服务提供的`key`配置到插件中，即可运行起来服务，而不需要去考虑各个服务间的交互，也不需要考虑开发板和服务之间的交互，你只需要做好你的机器人~

本项目服务端代码基于 Nodejs，硬件代码基于 Arduino IDE。

开源不易，右上角点个 `Star` 支持一下下吧 ~

# 交流群
QQ 交流群: 854445223

# 详细使用教程
[将硬件接入大语言模型(LLM)将变得如此简单~](https://juejin.cn/post/7384704245495234594)

# 怎么运行这个项目？

**简单三步，即可将一个智能助手运行起来。**

1. 安装服务端插件
2. 将插件引入你的项目中，配置好相关的`key`并运行
3. 将提供的客户端代码烧录到 `ESP32s3` 开发板中(需将/client/libraries 中的插件导入到IDE)

# 服务端所需环境

nodejs >= v14.0.0 

# 安装
新建一个目录，或者在你的项目目录中执行下面的安装操作。

使用 npm 安装
```
npm i esp-ai
```
或者使用 yarn 安装
```
yarn add esp-ai
```

# 运行前准备
1. 注册讯飞开放平台账号
2. 获取讯飞开放平台账号的 appid 和 apikey

**llm 也可以注册阿里积灵的来使用**

# 运行服务

在你的项目中，或者新增一个 index.js 并且写入以下代码。

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
执行下面代码即可运行服务
```
node ./index.js
```

生成环境中请使用 pm2 来运行服务以保证服务的可靠和性能。
```
pm2 start ./index.js -i max
```

# 客户端(ESP32)运行

客户端使用 `Arduino IDE` 编写，请先自行安装 `Arduino IDE`

### 头文件

需要提前将`/client/libraries`中的文件下载放到`Arduino IED`的依赖目录中，这个目录默认在：`C:\Users\用户名\Documents\Arduino\libraries`

除此之外，还需要提前安装好 `esp32` 开发板环境。
 
| 文件名 | 备注 |
| -------- | ------- |
| xiao_ming_tong_xue_inferencing      | 离线识别的模型     |
| arduino-audio-tool      | 最新版IDE可以直接搜索安装     |
| arduinoWebSockets      | 最新版IDE可以直接搜索安装     | 

### 代码上传

1. 将 /client/client.ino 下载到本地
2. 用 `Arduino IDE` 打开文件并且上传到板子中(注意修改相关的`wifi`和`服务IP`和`引脚配置`)。
3. 点击上传

# 客户端接线
## ESP32-s3 开发板
### INMP441(麦克风) 接线
| ESP32-s3 | INMP441 |
| -------- | ------- |
| 3v3      | VDD     |
| GND      | GND     |
| GND      | L/R     |
| 5        | WS      |
| 4        | SCK     |
| 6        | SD      |

### Max98357A(放大器) 接线
| ESP32-s3 | Max98357A |
| -------- | --------- |
| 3v3      | VDD       |
| GND      | GND       |
| 17       | LRC       |
| 16       | BCLK      |
| 15       | DIN       |

### 电位器(音量调节) 接线（可选，不接也行）
| ESP32-s3 | 电位器 |
| -------- | ------ |
| 3v3      | VDD    |
| GND      | GND    |
| OUT      | 34     |

### LED 接线（可选，不接也行）
| ESP32-s3 | LED  |
| -------- | ---- |
| GND      | GND  |
| 18       | 正极 |

# 支持的开发板

**✔️ 已支持**   **❗开发中**   **❌ 不支持**
 
| 开发板        | IAT | LLM | TTS | 离线唤醒 |
| ------------- | --- | --- | --- | -------- |
| ESP32-s3      | ✔️   | ✔️   | ✔️   | ✔️        |
| nodemcu-32    | ✔️   | ✔️   | ✔️   | ❌        |
| ESP32-xiao-s3 | ❗   | ❗   | ❗   | ✔️        |
| 增加中...     |


# 支持的平台
**✔️ 已支持**   **❗开发中**   **❌ 不支持**
 
| 服务方           | IAT | LLM | TTS |
| ---------------- | --- | --- | --- |
| 讯飞             | ✔️   | ✔️   | ✔️   |
| 百度             | ❗   | ❗   | ❗   |
| 本地服务         | ❗   | ❗   | ❗   |
| 阿里积灵(千问等) | ✔️   | ✔️   | ✔️   |
| 增加中...        |


# 离线唤醒方案
**✔️ 已支持**   **❗开发中**   **❌ 不支持**
 
| 方案         | 是否支持 | 模型开源仓库                                 |
| ------------ | -------- | -------------------------------------------- |
| Edge Impulse | ✔️        | https://studio.edgeimpulse.com/studio/422422 |
| ESP-SR       | ❗        | ❗                                            |
| 增加中...    |          |

# 实现原理步骤

INMP441(MIC) -> audio -> ESP32 -> wake-on-voice -> audio -> esp-ai -> IAT Server -> text -> esp-ai -> LLM -> reply -> esp-ai -> TTS -> esp-ai -> ESP32 -> max98357(loudspeaker)

待补充结构图

 
# 配置项
提供 `Typescript` 定义文件，`TS` 项目也可正常引用。

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

# 其他说明
目前语音唤醒由于训练样本不多，准确度并不高，会持续优化。

客户端代码目前不是以头文件提供，下个版本可能会改为头文件方式。

欢迎 pr 一起共建~。
