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
const config = {
    /**
     * 服务端口, 默认 8088
    */
    port: 8088,

    /**
     * 日志输出模式：0 不输出(线上模式)， 1 普通输出， 2 详细输出
    */
    devLog: 1,

    llm_qa_number: 5,

    /**
     * 为了保证服务的灵活，所以 IAT/TTS/LLM 都需要分别配置key。 哪怕都用的讯飞，也需要分别配置。
     * IAT 语音识别服务、TTS服务、LLM 服务的提供方, 默认为 xun_fei
    */
    gen_client_config: async (params) => {
        return {

            // IAT 语音识别服务
            iat_server: "xun_fei", 

            // TTS 文字转语音服务
            tts_server: "xun_fei", 

            // LLM 大语言服务
            llm_server: "xun_fei", 
            
            /**
             * 意图表：当用户唤醒 小明同学 后，小明同学可以做下面的任务
            */
            intention: [ 
                {
                    // 关键词
                    key: ["退下吧", "退下"],
                    // 内置的睡眠指令
                    instruct: "__sleep__",
                    message: "我先退下了，有需要再叫我。"
                }
            ],

        }
    },
 
}

module.exports = config;
