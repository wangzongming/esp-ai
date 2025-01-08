
const colors = require('colors');
const moment = require('moment');
function info(text, styles = []) {
    const { logs = {}} = G_config;
    const bold = styles?.includes?.('bold');
    text = text.cyan;
    if (bold) { text = text.bold }
    if (logs.info) {
        logs.info(text);
        return;
    };
    console.log(text);
}

function time() {
    return moment().format("HH:mm:ss SSS");
}
function t_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [Info] ${text.join(' ')}`.cyan);
        return;
    };
    console.log(`${time()} [Info] ${text.join(' ')}`.cyan);
}
function t_red_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [Info] ${text.join(' ')}`.magenta);
        return;
    };
    console.log(`${time()} [Info] ${text.join(' ')}`.magenta);
}
function tts_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [TTS] ${text.join(' ')}`.green);
        return;
    };
    console.log(`${time()} [TTS] ${text.join(' ')}`.green);
}

function iat_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [IAT] ${text.join(' ')}`.blue);
        return;
    };
    console.log(`${time()} [IAT] ${text.join(' ')}`.blue);
}


function llm_info(...text) {
    const { logs = {} } = G_config;
    if (logs.info) {
        logs.info(`${time()} [LLM] ${text.join(' ')}`.yellow);
        return;
    };
    console.log(`${time()} [LLM] ${text.join(' ')}`.yellow);
}

function error(text, styles = []) {
    const { logs = {} } = G_config;
    const bold = styles?.includes?.('bold');
    text = text.red;
    if (bold) { text = text.bold }

    if (logs.info) {
        logs.error(`${time()} ❌ [Error] ${text}`);
        return;
    };
    console.log(`❌ ${text}`);
}
 
module.exports = {
    info,
    t_info,
    t_red_info,
    error,
    tts_info,
    iat_info,
    llm_info
}