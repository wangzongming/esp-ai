 /** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const CryptoJS = require('crypto-js')

/**
 * @param {"IAT" | "TTS" | "LLM" } type 服务类型
*/
function getServerURL(type) {
    const { iat_server, llm_server, tts_server } = G_config;
    switch (type) {
        case "IAT":
            return getServersURL[iat_server](type);
        case "TTS":
            return getServersURL[tts_server](type);
        case "LLM":
            return getServersURL[llm_server](type);
        default:
            break;
    }
}

const getServersURL = {
    xun_fei,
    bai_du,
}
function xun_fei(type) {
    const { iat_server, api_key } = G_config;

    let url = "";
    // 获取当前时间 RFC1123格式
    const date = (new Date().toUTCString())
    const { apiKey, apiSecret, llm = "v3.5" } = api_key[iat_server];
    const config = {
        // IAT
        hostUrl: "wss://iat-api.xfyun.cn/v2/iat",
        host: "iat-api.xfyun.cn",
        uri: "/v2/iat",
        // TTS
        tts_hostUrl: "wss://tts-api.xfyun.cn/v2/tts",
        tts_hostUrlhost: "tts-api.xfyun.cn",
        tts_uri: "/v2/tts",
        // LLM
        llm_hostUrl: `wss://spark-api.xf-yun.com/${llm}/chat`,
        llm_host: "spark-api.xf-yun.com",
        llm_uri: `/${llm}/chat`,
        apiKey: apiKey,
        apiSecret: apiSecret,
    }
    function getAuthStr({ host, date, uri }) {
        let signatureOrigin = `host: ${host}\ndate: ${date}\nGET ${uri} HTTP/1.1`
        let signatureSha = CryptoJS.HmacSHA256(signatureOrigin, config.apiSecret)
        let signature = CryptoJS.enc.Base64.stringify(signatureSha)
        let authorizationOrigin = `api_key="${config.apiKey}", algorithm="hmac-sha256", headers="host date request-line", signature="${signature}"`
        let authStr = CryptoJS.enc.Base64.stringify(CryptoJS.enc.Utf8.parse(authorizationOrigin))
        return authStr
    }

    switch (type) {
        case "IAT":
            url = `${config.hostUrl}?authorization=${getAuthStr({ host: config.host, date, uri: config.uri })}&date=${date}&host=${config.host}`;
            break;
        case "TTS":
            url = `${config.tts_hostUrl}?authorization=${getAuthStr({ host: config.tts_host, date, uri: config.tts_uri })}&date=${date}&host=${config.tts_host}`;
            break;
        case "LLM":
            url = `${config.llm_hostUrl}?authorization=${getAuthStr({ host: config.llm_host, date, uri: config.llm_uri })}&date=${date}&host=${config.llm_host}`;
            break;
        default:
            break;
    }

    return url;
}

function bai_du(type) {
    // ing...
}

module.exports = getServerURL;