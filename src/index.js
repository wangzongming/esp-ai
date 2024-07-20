 
const log = require("./utils/log");
const package = require("../package.json");

/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
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
    log.info(LOGO);

    /**
     * [UUID, { 
     *       ws:{}, 
     *       runing: false,                   // 小明同学运行中  
     *       tts_list: new Map(ws),           // TTS 任务队列
     *       await_out_tts: [],               // 待播放任务列表
     *       await_out_tts_ing: false,        // 待播放任务正在执行
     *       await_out_tts_run: ()=> {},      // 执行播放
     *       iat_server_connected: false,  
     *       iat_ws: {},  
     *       iat_end_queue: ()=> void,        // iat 结束后的任务
     *       llm_ws: {},  
     *       llm_historys: [],                // 历史对话
     *       first_session: true              // 第一次的 会话
     *       client_out_audio_ing: false,     // 客户端是否还在播放音频
     *       iat_end_frame_timer: null,       // 最后一帧发送倒计时
     *       send_pcm: (pcm)=> void           // 音频发送函数
     *       start_iat: ()=> void             // 音频发送函数
     *       add_audio_out_over_queue: ()=> Promise<void>  // 音频流播放完毕的任务队列
     * }]
    */
    global.G_devices = new Map();
    
    const init_server = require("./functions/init_server")
    const _config = IS_DEV ? require("./config_dev") : require("./config")

    // 计算规则：buffer_count * (buffer_size / 2) = 8 * 512 = 4096
    global.G_max_audio_chunk_size = 4096;

    global.G_ws_server = null;
    global.G_config = { ..._config, ...config };

    // IAT 静默时间
    global.G_vad_eos = G_config.vad_eos || 2500;
 
    G_ws_server = init_server();
}

IS_DEV && main({});

module.exports = main;