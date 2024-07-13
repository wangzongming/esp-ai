
const colors = require('colors');
function info(text, styles = []) {
    const bold = styles.includes('bold');
    text = text.cyan;
    if (bold) { text = text.bold }
    console.log(text);
}

function error(text, styles = []) {
    const bold = styles.includes('bold');
    text = text.red;
    if (bold) { text = text.bold }
    console.log(`❌ ${text}`);
}

// cyan.bold
module.exports = {
    info,
    error
}