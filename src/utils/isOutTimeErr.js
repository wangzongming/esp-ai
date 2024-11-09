
function isOutTimeErr(errText) { 
    return `${errText}`.indexOf("connect ECONNREFUSED") !== -1 || `${errText}`.indexOf("connect ETIMEDOUT") !== -1;
}
module.exports = isOutTimeErr;
