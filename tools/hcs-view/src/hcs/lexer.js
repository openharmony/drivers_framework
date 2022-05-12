/*
* Copyright (c) 2022 Shenzhen Kaihong Digital Industry Development Co., Ltd. 
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at 
*
* http:// www.apache.org/licenses/LICENSE-2.0 
*
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License. 
*/
const { XMessage } = require("../message/XMessage");
const { NapiLog } = require("./NapiLog")

function code(s) {
    return s.charCodeAt(0)
}

function isSpace(c) {
    return c == code(' ') || c == code('\t') || c == code('\r');
}

function isalpha(c) {
    if (code('a') <= c[0] && c[0] <= code('z')) return true;
    if (code('A') <= c[0] && c[0] <= code('Z')) return true;
    return false;
}

function isalnum(c) {
    return isalpha(c) || isNum(c)
}

function isNum(c) {
    return code('0') <= c[0] && c[0] <= code('9')
}

function toStr(c) {
    return String.fromCharCode(c[0])
}

class TokenType { }
TokenType.NUMBER = 256;
TokenType.TEMPLATE = 257;
TokenType.LITERAL = 258;
TokenType.STRING = 259;
TokenType.REF_PATH = 260;
TokenType.FILE_PATH = 261;
TokenType.ROOT = 262;
TokenType.INCLUDE = 263;
TokenType.DELETE = 264;
TokenType.BOOL = 265;
TokenType.EOF = -1

class Lexer {
    constructor() {
        this.keyWords_ = {
            "#include": TokenType.INCLUDE,
            "root": TokenType.ROOT,
            "delete": TokenType.DELETE,
            "template": TokenType.TEMPLATE,
        };
    }

    initialize(sourceName) {
        this.srcName_ = sourceName
        if (!(sourceName in Lexer.FILE_AND_DATA)) {
            XMessage.gi().send("getfiledata", sourceName);
            return false;
        }
        this.data_ = Lexer.FILE_AND_DATA[sourceName]
        NapiLog.logError("------------data start----------------");
        NapiLog.logError(sourceName);
        NapiLog.logError(Lexer.FILE_AND_DATA[sourceName])
        NapiLog.logError("------------data end------------------");

        this.bufferStart_ = 0;
        this.bufferEnd_ = this.data_.length - 1;
        this.lineno_ = 1;
        this.lineLoc_ = 1;

        return true;
    }

    lexInclude(token) {
        this.consumeChar();
        this.lexFromLiteral(token);
        if (token.strval != "include") {
            return false;
        }

        token.type = TokenType.INCLUDE;
        return true;
    }

    isConsumeCharCode(c) {
        if (c[0] == code(';') || c[0] == code(',') || c[0] == code('[') || c[0] == code(']')
            || c[0] == code('{') || c[0] == code('}') || c[0] == code('=') || c[0] == code('&')
            || c[0] == code(':')) {
            return true;
        }
        return false;
    }

    lex(token) {
        let c = [];
        this.initToken(token);
        do {
            if (!this.peekChar(c, true)) {
                token.type = TokenType.EOF;
                return true;
            }
            if (c[0] == code('#')) {
                return this.lexInclude(token);
            }
            if (isalpha(c)) {
                this.lexFromLiteral(token);
                return true;
            }

            if (isNum(c)) {
                return this.lexFromNumber(token);
            }

            if (c[0] == code('/')) {
                if (!this.processComment()) {
                    return false;
                }
            } else if (this.isConsumeCharCode(c)) {
                this.consumeChar();
                token.type = c[0];
                token.lineNo = this.lineno_;
                break;
            } else if (c[0] == code('"')) {
                return this.lexFromString(token);
            } else if (c[0] == code('+') || c[0] == code('-')) {
                return lexFromNumber(token);
            } else if (c[0] == -1) {
                token.type = -1;
                break;
            } else{
                NapiLog.logError("can not recognized character '" + toStr(c) + "'" + this.bufferStart_);
                return false;
            }
        } while (true);
        return true;
    }

    peekChar(c, skipSpace) {
        if (skipSpace) {
            while (this.bufferStart_ <= this.bufferEnd_ && (isSpace(this.data_[this.bufferStart_]) ||
                this.data_[this.bufferStart_] == code('\n'))) {
                this.lineLoc_++;
                if (this.data_[this.bufferStart_] == code('\n')) {
                    this.lineLoc_ = 0;
                    this.lineno_++;
                }
                this.bufferStart_++;
            }
        }

        if (this.bufferStart_ > this.bufferEnd_) {
            return false;
        }
        c[0] = this.data_[this.bufferStart_];
        return true;
    }

    initToken(token) {
        token.type = 0;
        token.numval = 0;
        token.strval = "";
        token.src = this.srcName_;
        token.lineNo = this.lineno_;
    }

    lexFromLiteral(token) {
        let value = "";
        let c = [];

        while (this.peekChar(c, false) && !isSpace(c[0])) {
            if (!isalnum(c) && c != code('_') && c != code('.') && c != code('\\')) {
                break;
            }
            value += toStr(c);
            this.consumeChar();
        }

        do {
            if (value == "true") {
                token.type = TokenType.BOOL;
                token.numval = 1;
                break;
            } else if (value == "false") {
                token.type = TokenType.BOOL;
                token.numval = 0;
                break;
            }

            if (value in this.keyWords_) {
                token.type = this.keyWords_[value];
                break;
            }

            if (value.indexOf(".") >= 0) {
                token.type = TokenType.REF_PATH;
            } else {
                token.type = TokenType.LITERAL;
            }
        } while (false);


        token.strval = value;
        token.lineNo = this.lineno_;
    }
    getRawChar() {
        this.lineLoc_++;
        let ret = this.data_[this.bufferStart_];
        this.bufferStart_++;
        return ret
    }
    getChar(c, skipSpace) {
        let chr = this.getRawChar();
        if (skipSpace) {
            while (isSpace(chr)) {
                chr = this.getRawChar();
            }
        }

        if (chr == code('\n')) {
            this.lineno_++;
            this.lineLoc_ = 0;
        }
        c[0] = chr;
        return true;
    }
    consumeChar() {
        let c = []
        this.getChar(c, false);
    }

    lexFromOctalNumber(c, param) {
        while (this.peekChar(c) && isNum(c)) {
            this.consumeChar();
            param.value += toStr(c);
        }
        param.v = parseInt(param.value, 8)
        param.baseSystem = 8;
    }

    lexFromHexNumber(c, param) {
        this.consumeChar();
        while (this.peekChar(c, false) && (isNum(c) || (c[0] >= code('a') && c[0] <= code('f'))
            || (c[0] >= code('A') && c[0] <= code('F')))) {
            param.value += toStr(c);
            this.consumeChar();
        }
        param.v = parseInt(param.value, 16)
        param.baseSystem = 16;
    }

    lexFromBinaryNumber(c, param) {
        this.consumeChar();
        while (this.peekChar(c, false) && (c[0] == code('0') || c[0] == code('1'))) {
            param.value += toStr(c);
            this.consumeChar();
        }
        param.v = parseInt(param.value, 2);
        param.baseSystem = 2;
    }

    lexFromNumber(token) {
        let c = [];

        let errno = 0;
        let param = {
            value: "",
            v: 0,
            baseSystem: 10
        }

        this.getChar(c, false);
        switch (c[0]) {
            case code('0'):
                if (!this.peekChar(c, true)) {
                    break;
                }
                if (isNum(c)) {
                    this.lexFromOctalNumber(c, param);
                    break;
                }
                if (c[0] == code('x') || code('x') == code('X')) {
                    this.lexFromHexNumber(c, param);
                    break;
                } else if (c[0] == code('b')) {
                    this.lexFromBinaryNumber(c, param);
                    break;
                }
                break;
            case code('+'): // fall-through
            case code('-'): // fall-through, signed decimal number
            default: // unsigned decimal number
                param.value += toStr(c);
                while (this.peekChar(c, true) && isNum(c)) {
                    this.consumeChar();
                    param.value += toStr(c);
                }
                param.v = parseInt(param.value, 10)
                param.baseSystem = 10
                break;
        }

        if (errno != 0) {
            NapiLog.logError("illegal number: " + param.value)
            return false;
        }
        token.type = TokenType.NUMBER;
        token.numval = param.v;
        token.lineNo = this.lineno_;
        token.baseSystem = param.baseSystem;
        return true;
    }

    lexFromString(token) {
        let c = [];
        this.getChar(c, false); // skip first '"'
        let value = "";
        while (this.getChar(c, false) && c[0] != code('"')) {
            value += toStr(c);
        }

        if (c != code('"')) {
            NapiLog.logError("unterminated string");
            return false;
        }
        token.type = TokenType.STRING;
        token.strval = value;
        token.lineNo = this.lineno_;
        return true;
    }
    processComment() {
        let c = [];
        this.consumeChar();// skip first '/'
        if (!this.getChar(c, true)) {
            NapiLog.logError("unterminated comment");
            return false;
        }

        if (c[0] == code('/')) {
            while (c[0] != code('\n')) {
                if(!this.getChar(c, true)) break;
            }
            if (c[0] != code('\n') && c[0] != TokenType.EOF) {
                NapiLog.logError("unterminated signal line comment");
                return false;
            }
        } else if (c[0] == code('*')) {
            while (this.getChar(c, true)) {
                if (c[0] == code('*') && this.getChar(c, true) && c[0] == code('/')) {
                    return true;
                }
            }
            if (c[0] != code('/')) {
                NapiLog.logError("unterminated multi-line comment");
                return false;
            }
        } else {
            NapiLog.logError("invalid character");
            return false;
        }

        return true;
    }
}
Lexer.FILE_AND_DATA = {};

module.exports = {
    Lexer,
    TokenType,
    code
}