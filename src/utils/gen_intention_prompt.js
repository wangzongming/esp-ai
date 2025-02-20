function gen_intention_prompt(intention = []){
    const arrKey = intention.filter(item => Array.isArray(item.key)); 
    const strKey = arrKey.map(item => `当用户要求${item.key.join("或")}时，回复"${item.message || "好的"}"`).join(", "); 
    return [
        {
            role: "system",  
            content: `${strKey}`
        }
    ];
}
module.exports = gen_intention_prompt