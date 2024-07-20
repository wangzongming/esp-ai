
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

function tts_info(...text){
    console.log(`${time()} [TTS] ${text.join(' ')}`.green);
}

function iat_info(...text){
    console.log(`${time()} [IAT] ${text.join(' ')}`.blue);
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
    error,
    tts_info,
    iat_info,
    llm_info
}