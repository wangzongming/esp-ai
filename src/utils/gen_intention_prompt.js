function gen_intention_prompt(intention = []){
    const arrKey = intention.filter(item => Array.isArray(item.key));  
    const strKey = `你拥有以下能力：【${arrKey.map(item=> item.key.map(_item=> item?.message ? _item + '->' + item?.message : _item).join("、")) }】。当用户有意要求执行上述功能时，仅需用简短语句表达帮其操作的意愿（例如：' 好的，马上帮你开灯～'、'这就给你唱歌～'）如果有 -> 说明需要固定回复指定的语句，无需解释具体实现步骤、扩展功能细节或追问用户。所有功能的实际执行由第三方服务完成，你的回复需严格限定在确认操作意向的范围内，不包含其他无关内容。`
    return [
        {
            role: "system",  
            content: `${strKey}`
        }
    ];
}
module.exports = gen_intention_prompt