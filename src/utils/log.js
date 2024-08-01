
const colors = require('colors');
const moment = require('moment');
function info(text, styles = []) {
    const bold = styles.includes('bold');
    text = text.cyan;
    if (bold) { text = text.bold }
    console.log(text);
} 

function time() {
    return moment().format("HH:mm:ss");
}
function t_info(...text) {
    console.log(`${time()} [Info] ${text.join(' ')}`.cyan);
} 
function t_red_info(...text) {
    console.log(`${time()} [Info] ${text.join(' ')}`.magenta);
} 
function tts_info(...text){
    console.log(`${time()} [TTS] ${text.join(' ')}`.green);
}

function iat_info(...text){
    console.log(`${time()} [IAT] ${text.join(' ')}`.white);
}


function llm_info(...text){
    console.log(`${time()} [LLM] ${text.join(' ')}`.yellow);
}

function error(text, styles = []) {
    const bold = styles.includes('bold');
    text = text.red;
    if (bold) { text = text.bold }
    console.log(`❌ ${text}`);
}

// cyan.bold
module.exports = {
    info,
    t_info,
    t_red_info,
    error,
    tts_info,
    iat_info,
    llm_info
}