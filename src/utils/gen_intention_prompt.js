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

function gen_intention_prompt(intention = []){
    const arrKey = intention.filter(item => Array.isArray(item.key));  
    const fae = arrKey.map(item=> item.key.map(_item=> item?.message ? _item + '->' + item?.message : _item).join("、"));
    const strKey = `你具备以下功能：【${fae}】。请注意：仅当**用户“最新一条消息”明确发出上述请求**时，才需要触发功能响应。比如“唱个歌吧”“帮我关灯”“我要测试测试”，这些属于明确请求。当触发功能时，只回复一句简短温柔的话确认即可（例如：“这就给你唱歌～” 或 “好的，我马上帮你开灯～”）。不要重复回复已经说过的内容，也不要在用户没有明确请求时误触功能。如果用户在闲聊（如“你好呀”“你在干嘛”），请不要触发任何功能，改用用户指定的人设或智能助手自然聊天。不要根据之前的对话重复功能响应，每次都以用户最新一句话为准。`
    return [
        {
            role: "system",  
            content: `${strKey}`
        }
    ];
}
module.exports = gen_intention_prompt