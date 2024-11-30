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

const log = require("./utils/log");
const package = require("../package.json");
const EspAiInstance = require("./instance");

global.G_config = {};
const LOGO = `  

 ********  ******** *******            **     **
/**/////  **////// /**////**          ****   /**
/**      /**       /**   /**         **//**  /**
/******* /*********/*******  *****  **  //** /**
/**////  ////////**/**////  /////  **********/**
/**             /**/**            /**//////**/**
/******** ******** /**            /**     /**/**
//////// ////////  //             //      // // 

服务端版本号：v${package.version}
`
const IS_DEV = process.argv[2] === "--dev";
function main(config = {}) {
    try {
        log.info(LOGO);

        /**
         * [UUID, { 
         *       ws:{}, 
         *       user_config: {  // 用户配置
         *          
         *       },                
         *       runing: false,                   // 小明同学运行中  
         *       tts_list: new Map(ws),           // TTS 任务队列
         *       await_out_tts: [],               // 待播放任务列表
         *       await_out_tts_ing: false,        // 待播放任务正在执行
         *       await_out_tts_run: ()=> {},      // 执行播放
         *       iat_server_connect_ing: false,
         *       iat_server_connected: false,  
         *       iat_ws: {},  
         *       iat_end_queue: ()=> void,        // iat 结束后的任务
         *       llm_ws: {},  
         *       llm_historys: [],                // 历史对话
         *       first_session: true,             // 第一次的 会话
         *       session_id: "",                  // 会话id
         *       client_out_audio_ing: false,     // 客户端是否还在播放音频
         *       iat_end_frame_timer: null,       // 最后一帧发送倒计时
         *       send_pcm: (pcm)=> void           // 音频发送函数
         *       start_iat: ()=> void             // 音频发送函数
         *       add_audio_out_over_queue: ()=> Promise<void>  // 音频流播放完毕的任务队列
         * }]
        */
        global.G_devices = new Map();
        const Instance = new EspAiInstance()
        global.G_Instance = Instance;


        const init_server = require("./functions/init_server")
        const _config = IS_DEV ? require("./config_dev") : require("./config")   
        global.G_max_audio_chunk_size = 1280 * 5;      
        // global.G_max_buffered_amount = 1024 * 4;

        global.G_ws_server = null;
        global.G_config = { ..._config, ...config };

        // 缓存 TTS 下个版本在做 ing...
        global.G_cahce_TTS = new Map();
        global.G_cahce_TTS_number = (G_config.cache_TTS_number || G_config.cache_TTS_number === 0) ? G_config.cache_TTS_number : 1000; // 缓存 TTS 数量。
        global.G_get_cahce_TTS = (text) => G_cahce_TTS.get(text);
        global.G_set_cahce_TTS = (text, audio) => {
            if (G_cahce_TTS.size > G_cahce_TTS_number) return;
            G_cahce_TTS.set(text, audio);
        };

        log.info(`服务端口：${G_config.port}`);
        log.info(`服务插件：${G_config.plugins ? G_config.plugins.map(item => item.name).join(" | ") : "-"}`);

        G_ws_server = init_server();

        return Instance
    } catch (err) {
        console.log(err);
        log.error(`服务运行失败。`);
    }
}

IS_DEV && main({});

module.exports = main;