
/**
 * Copyright (c) 2024 小明IO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Commercial use of this software requires prior written authorization from the Licensor.
 * 请注意：将 ESP-AI 代码用于商业用途需要事先获得许可方的授权。
 * 删除与修改版权属于侵权行为，请尊重作者版权，避免产生不必要的纠纷。
 * 
 * @author 小明IO   
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */

/**
 * 通用规则：
 * 0xx 都是服务相关错误
 * 1xx 都是 IAT 相关错误
 * 2xx 都是 LLM 相关错误
 * 3xx 都是 TTS 相关错误
 * 
 * 错误码对照表：
 * 自定义认证服务时也务必遵循以下错误码协议
 * 
 *  错误码  |  错误信息
 * ------------------
 *  000    |  未知服务错误
 *  001    |  服务内部错误
 *  002    |  服务端认证错误
 *  003    |  获取服务信息失败，说明 api_key 有问题
 *  004    |  连接服务时没传入 device_id
 *  005    |  [业务预留] device_id 有问题
 *  006    |  获取服务端配置失败 gen_client_config 配置返回 false 时
 *  007    |  网络错误，一般是服务端未启动
 * 
 *  100    |  未知 IAT 服务错误
 *  101    |  未知 IAT 服务连接失败
 *  102    |  未知 IAT 服务调用错误
 * 
 *  200    |  未知 LLM 服务错误
 *  201    |  未知 LLM 服务连接失败
 *  202    |  未知 LLM 服务调用错误
 * 
 *  300    |  未知 TTS 服务错误
 *  301    |  未知 TTS 服务连接失败
 *  302    |  未知 TTS 服务调用错误
 * 
 *  4000   |  ESP-AI 开放平台超体被禁用，或者已经删除
 *  4001   |  ESP-AI 开放平台额度卡不存在
 *  4002   |  ESP-AI 开放平台额度不足
 * 
*/
function error_catch_hoc(ws) {
    return (type, code, message) => {
        ws && ws.send(JSON.stringify({
            type: "error",
            at: type,
            code,
            message
        }));
    }
}
module.exports = error_catch_hoc