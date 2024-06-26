/** 
 * @author xiaomingio 
 * @github https://github.com/wangzongming/esp-ai  
 */
const { v4: uuidv4 } = require('uuid');

const createUUID = () => {
    return uuidv4().replace(/(-)/g, '');
}

module.exports = createUUID;