
/**
 * 服务负载测试
*/


const WebSocket = require('ws');
const colors = require('colors');
const fs = require('fs');
const path = require('path');

/***
 * 以下测试数据以腾讯云为服务商：cpu 2核 | 内存 2G | 带宽 4mb |  SSD 50GB
 * 测试过程中服务端开了日志输出，实际性能可以略微在高一点点
 * 
 * 1、连接+发送一条数据，测试情况：(不考虑重新连接是否能连接上，只记录并发请求的情况)
 * --------------------------------------------------------------------------------
 *   连接数量   |  成功量 | 失败量  | 并发瞬间服务情况            | 连接后服务情况  
 * ---------------------------------------------------------------------------------
 *   1000      |  1000  |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   2000      |  2000  |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   3000(最大) |  3000  |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   4000      |  3806  |194(5%)| cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   5000      |  4685  |315(6.7%)| cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   6000      |  4030  |1970(32%)| cpu:100%,100%, mem:1.6GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   10000     |  52    |服务蹦了   | -                         | -
 * ---------------------------------------------------------------------------------
 * 
 * 
 * 2、连接+一些必要标志服+发送音频流，测试情况：10kb音频流，2048字节分片发送。每个连接应发消息数量 6 次
 *  * --------------------------------------------------------------------------------
 *   连接数量   |连接成功量| 应发消息 | 实发消息  | 失败量  | 并发瞬间服务情况            | 连接后服务情况  
 * ---------------------------------------------------------------------------------
 *   100      |  922   | 600     |600      |  0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   500      |  500   | 3000    | 3000    |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   1000     |  1000   | 6000    | 6000    |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   2000(最大)     |  2000   | 12000    | 12000    |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 *   3000     |  2982   | 18000    | 2982    |   0   | cpu:100%,100%, mem:1.5GB  | cpu:4%, 3%, mem:1.5GB 
 * ---------------------------------------------------------------------------------
 * 
*/

// 压力测试参数
// const url = 'ws://192.168.3.3:8088?v=1.0.0';         // 替换为你的WebSocket服务器URL
const url = 'ws://101.34.59.36:8088?v=1.0.0';   // 替换为你的WebSocket服务器URL
const totalClients = 3000;                      // 总的WebSocket客户端数量
// const messagesPerClient = 100;                  // 每个客户端发送的消息数量
// const messageInterval = 100;                    // 每条消息之间的间隔时间（毫秒） 

// 统计数据
let connectedClients = 0;
let sentMessages = 0;
let receivedMessages = 0;



// 模拟上传音频的流测试文件
// 音频PCM: 你好，这是一段测试音频，用来测试压力测试措施
const filePath = path.join(__dirname, './src/functions/client_messages/output.bin');
const readStream = fs.createReadStream(filePath);
let fileContent = Buffer.alloc(0);;
readStream.on('data', (chunk) => {
    fileContent = Buffer.concat([fileContent, chunk]); 
});
readStream.on('end', () => {
    console.log('测试音频流文件大小:', fileContent.length / 1024 + "KB");
    // console.log(fileContent); 
    // 创建并发WebSocket客户端
    for (let i = 0; i < totalClients; i++) {
        createClient(i);
    }

    // 定期打印统计信息
    setInterval(() => {
        console.log(`====================================`);
        console.log(`连接总数: ${connectedClients}`);
        console.log(`发送消息总数: ${sentMessages}`);
        console.log(`接收消息总数: ${receivedMessages}`);
        console.log(`====================================`);
    }, 1000);
});
readStream.on('error', (err) => {
    console.error('测试音频流文件读取出错:', err);
});

const all_wss = [];
function createClient(id) {
    const ws = new WebSocket(url);

    ws.on('open', () => {
        connectedClients++;
        console.log(`客户端 ${id} 已连接. 总链接数量: ${connectedClients}`);
        // 连接负载测试
        sentMessages++;
        // 流消息, 模拟发送一片流 
        ws.send(`{"type":"play_audio_ws_conntceed"}`);
        // 延时消息
        ws.send(`{"type":"cts_time","stc_time":123456789454,"id":${id}}`);
        all_wss.push(ws);
        if (connectedClients === totalClients) {
            setTimeout(() => {
                console.log(`开始音频流上传并发测试`.green);
                // 并发流测试
                for (let i = 0; i < all_wss.length; i++) {
                    const ws = all_wss[i];
                    ws.send(`{"type":"start"}`); 
                    for (let i = 0; i < fileContent.length; i += 2048) {
                        const end = Math.min(i + 2048, fileContent.length);
                        const chunk = fileContent.slice(i, end);
                        // console.log(chunk)
                        sentMessages++;
                        ws.send(chunk);
                    }
                }
            }, 10000)
        }

        // 发送消息负载测试
        // let messageCount = 0;
        // const interval = setInterval(() => {
        //     if (messageCount < messagesPerClient) {
        //         ws.send(`{"type":"cts_time","stc_time":123456789454,"id":${id}}`);
        //         sentMessages++;
        //         // console.log(`客户端 ${id} 发送消息 ${messageCount + 1}`);
        //         messageCount++;
        //     } else {
        //         clearInterval(interval);
        //         // ws.close();
        //     }
        // }, messageInterval);
    });

    ws.on('message', (message) => {
        receivedMessages++;
        // console.log(`客户端 ${id} 收到消息: ${message}`);
    });

    ws.on('close', () => {
        connectedClients--;
        console.log(`客户端 ${id} 断开连接. 总连接客户端数量: ${connectedClients}`.red);
    });

    ws.on('error', (err) => {
        console.error(`客户端 ${id} 碰到错误: ${err.message}`.red);
    });
}

