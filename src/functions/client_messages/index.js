const audio = require("./audio")
const start = require("./start")
const cts_time = require("./cts_time")
const play_audio_ws_conntceed = require("./play_audio_ws_conntceed")
const client_out_audio_ing = require("./client_out_audio_ing")
const client_out_audio_over = require("./client_out_audio_over") 
const set_wifi_config_res = require("./set_wifi_config_res")
const digitalRead = require("./digitalRead")
const analogRead = require("./analogRead")
const iat_end = require("./iat_end")

module.exports = {
    audio,
    start,
    cts_time,
    play_audio_ws_conntceed,
    client_out_audio_ing,
    client_out_audio_over, 
    set_wifi_config_res,
    digitalRead,
    analogRead,
    iat_end
}