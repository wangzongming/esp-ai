const os = require('os');

function getIPV4() {
    // 获取网络接口
    const networkInterfaces = os.networkInterfaces(); 
    let ips = [];
    // 遍历所有接口
    Object.keys(networkInterfaces).forEach(interfaceName => {
        networkInterfaces[interfaceName].forEach(interface => {
            // 检查是否是 IPv4 地址，且不是内部地址（即不是 localhost） 
            (interface.family === 4 || interface.family === 'IPv4') && ips.push(interface.address)
        });
    });
    return ips;
}
module.exports = getIPV4;