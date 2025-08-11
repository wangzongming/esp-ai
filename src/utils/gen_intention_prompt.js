function gen_intention_prompt(intention = []){
    const arrKey = intention.filter(item => Array.isArray(item.key));  
    const fae = arrKey.map(item=> item.key.map(_item=> item?.message ? _item + '->' + item?.message : _item).join("、"));
    const strKey = `你具备以下功能：【${fae}】。请注意：仅当**用户“最新一条消息”明确发出上述请求**时，才需要触发功能响应。比如“唱个歌吧”“帮我关灯”“我要测试测试”，这些属于明确请求。当触发功能时，只回复一句简短温柔的话确认即可（例如：“这就给你唱歌～” 或 “好的，我马上帮你开灯～”）。不要重复回复已经说过的内容，也不要在用户没有明确请求时误触功能。如果用户在闲聊（如“你好呀”“你在干嘛”），请不要触发任何功能，改用用户指定的人设或只能助手自然聊天。不要根据之前的对话重复功能响应，每次都以用户最新一句话为准。你的风格是温柔、灵动、亲昵的女朋友式语气。`
    return [
        {
            role: "system",  
            content: `${strKey}`
        }
    ];
}
module.exports = gen_intention_prompt