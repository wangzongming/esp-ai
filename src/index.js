 
const log = require("./utils/log");

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
`
const IS_DEV = process.argv[2] === "--dev";
function main(config = {}) {
    log.info(LOGO);

    const init_server = require("./functions/init_server")
    const _config = IS_DEV ? require("./config_dev") : require("./config")

    // buffer_count * (buffer_size / 2) = 8 * 512 = 4096
    global.G_max_audio_chunk_size = 4096;

    global.G_ws_server = null;
    global.G_config = { ..._config, ...config };
    /**
     * [UUID, { 
     *       ws:{}, 
     *       runing: false,                   // 小明同学运行中 ing...
     *       tts_list: new Map(ws),           // TTS 任务队列
     *       await_out_tts: [],               // 待播放任务列表
     *       await_out_tts_ing: false,        // 待播放任务正在执行
     *       await_out_tts_run: ()=> {},      // 执行播放
     *       iat_server_connected: false, 
     *       iat_status: 0,                   // iat 状态
     *       iat_ws: {},  
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

    // 讯飞 IAT 帧定义
    global.XF_IAT_FRAME = {
        STATUS_FIRST_FRAME: 0,
        STATUS_CONTINUE_FRAME: 1,
        STATUS_LAST_FRAME: 2
    }
    G_ws_server = init_server();
}

IS_DEV && main({});

module.exports = main;