//Uses a ".cjs" file extension because "type": "module" in package.json
//"type": "module" uses ES Modules (import) instead of CommonJS modules (require)
const path = require('path');

module.exports = {
    entry: './src/index.js',
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'dist')
    },
    mode: 'production'
};