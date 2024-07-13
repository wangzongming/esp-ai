const os = require('os');

function getIPV4() {
    // 获取网络接口
    const networkInterfaces = os.networkInterfaces();
    let ip = "127.0.0.1";
    // 遍历所有接口
    Object.keys(networkInterfaces).forEach(interfaceName => {
        networkInterfaces[interfaceName].forEach(interface => {
            // 检查是否是 IPv4 地址，且不是内部地址（即不是 localhost）
            if (interface.family === 'IPv4' && !interface.internal) { 
                ip = interface.address;
            }
        });
    });
    return ip;
}
module.exports = getIPV4;