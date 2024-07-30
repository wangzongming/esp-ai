function parseUrlParams(url) { 
    const params = new URLSearchParams(url.split('?')[1]);
 
    const paramsObject = {};
    for (const [key, value] of params.entries()) {
        paramsObject[key] = value;
    }

    return paramsObject;
}
module.exports = parseUrlParams;