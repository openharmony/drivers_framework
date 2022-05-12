/*
* Copyright (c) 2022 Shenzhen Kaihong Digital Industry Development Co., Ltd. 
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at 
*
* http://www.apache.org/licenses/LICENSE-2.0 
*
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License. 
*/
const { NapiLog } = require("./NapiLog")
const { Lexer, TokenType, code } = require("./lexer")
const { AstObject, ConfigNode, ConfigTerm, ConfigArray, NodeRefType, ObjectType, Ast } = require("./ast")

function copy(obj) {
    return JSON.parse(JSON.stringify(obj));
}
class Parser {
    constructor() {
        this.lexer_ = new Lexer();
        this.current_ = {}
        this.srcQueue_ = []
        this.astList = {}
    }
    parse(fn) {
        if (this.srcQueue_.indexOf(fn) == -1) this.srcQueue_.push(fn)

        while (this.srcQueue_.length > 0) {
            let includeList = []
            let oneAst = this.parseOne(this.srcQueue_[0], includeList)
            if (oneAst == null) {
                return false;
            }

            this.astList[this.srcQueue_[0]] = {
                ast: oneAst,
                include: includeList,
            };
            this.srcQueue_.shift();
            this.srcQueue_ = this.srcQueue_.concat(includeList)
        }
        return this.astList;
        astList.push(astList.shift());
        this.ast_ = astList.shift();

        if (!this.ast_.merge(astList)) {
            NapiLog.logError("failed to merge ast");
            return false;
        }
        if (this.ast_.getAstRoot() == null) {
            NapiLog.logError(fn + ": Empty hcs file");
            return false;
        }

        if (!this.ast_.expand()) {
            return false;
        }
        return astList[0]
    }
    convertAbsPath(includeBy, includePath) {
        if (navigator.userAgent.toLowerCase().indexOf("win") > -1) {//windows
            if (includePath[1] != ':') {
                let currentSrc = includeBy.substring(0, includeBy.lastIndexOf("\\") + 1)
                includePath = currentSrc + includePath.replace(/\//g, "\\");

                includePath = includePath.replace(/\\\.\\/g, "\\");
            }
        }
        else if (includePath[0] != '/') {
            let currentSrc = includeBy.substring(0, includeBy.lastIndexOf("/") + 1)
            includePath = currentSrc + includePath
        }
        return includePath
    }
    processInclude(includeList) {
        do {
            if (!this.lexer_.lex(this.current_) || this.current_.type != TokenType.STRING) {
                NapiLog.logError(this.lexer_ + "syntax error, expect include path after ’#include‘");
                return false;
            }

            let includePath = this.current_.strval;
            if (includePath.length <= 0) {
                NapiLog.logError(this.lexer_ + "include invalid file:" + this.includePath);
                return false;
            }
            includePath = this.convertAbsPath(this.srcQueue_[0], includePath);

            includeList.push(includePath);

            if (!this.lexer_.lex(this.current_)) {
                return false;
            }

            if (this.current_.type != TokenType.INCLUDE) {
                break;
            }
        } while (true);

        return true;
    }
    parseOne(src, includeList) {
        if (!this.lexer_.initialize(src)) {
            return null;
        }

        if (!this.lexer_.lex(this.current_)) {
            return null;
        }

        if (this.current_.type == TokenType.INCLUDE && !this.processInclude(includeList)) {
            return null;
        }

        let rootNode = null;
        if (this.current_.type == TokenType.ROOT) {
            let preToken = copy(this.current_);
            preToken.type = TokenType.LITERAL;
            preToken.strval = "root";
            rootNode = this.parseNode(preToken);
            if (rootNode == null) {
                return null;
            }
        } else if (this.current_.type != TokenType.EOF) {
            NapiLog.logError("syntax error, expect root node of end of file")
            return null;
        }

        if (!this.lexer_.lex(this.current_) || this.current_.type != TokenType.EOF) {
            NapiLog.logError("syntax error, expect EOF")
            return null;
        }

        let oneAst = new Ast(rootNode);
        return oneAst;
    }

    parseNode(name, bracesStart) {

        /* bracesStart： if true, current is '{' , else need to read next token and check with '}' */
        if (!bracesStart) {
            if (!this.lexer_.lex(this.current_) || this.current_.type != code('{')) {
                NapiLog.logError("syntax error, node miss '{'")
                return null;
            }
        }

        let node = new ConfigNode(name, NodeRefType.NODE_NOREF, "");
        let child;
        while (this.lexer_.lex(this.current_) && this.current_.type != 125) {
            switch (this.current_.type) {
                case TokenType.TEMPLATE:
                    child = this.parseTemplate();
                    break;
                case TokenType.LITERAL:
                    child = this.parseNodeAndTerm();
                    break;
                default:
                    NapiLog.logError("syntax error, except '}' or TEMPLATE or LITERAL for node '" + name.strval + '\'')
                    return null;
            }
            if (child == null) {
                return null;
            }

            node.addChild(child);
        }

        if (this.current_.type != code('}')) {
            NapiLog.logError(this.lexer_ + "syntax error, node miss '}'");
            return null;
        }
        return node;
    }

    parseNodeAndTerm() {
        let name = copy(this.current_);
        if (!this.lexer_.lex(this.current_)) {
            NapiLog.logError(this.lexer_ + "syntax error, broken term or node");
            return null;
        }

        switch (this.current_.type) {
            case code('='):
                return this.parseTerm(name);
            case code('{'):
                return this.parseNode(name, true);
            case code(':'):
                if (this.lexer_.lex(this.current_)) {
                    return this.parseNodeWithRef(name);
                }
                NapiLog.logError("syntax error, unknown node reference type");
                break;
            default:
                NapiLog.logError("syntax error, except '=' or '{' or ':'");
                break;
        }

        return null;
    }
    parseArray() {
        let array = new ConfigArray(this.current_);
        let arrayType = 0;

        while (this.lexer_.lex(this.current_) && this.current_.type != code(']')) {
            if (this.current_.type == TokenType.STRING) {
                array.addChild(new AstObject("", ObjectType.PARSEROP_STRING, this.current_.strval, this.current_));
            } else if (this.current_.type == TokenType.NUMBER) {
                array.addChild(new AstObject("", ObjectType.PARSEROP_UINT64,
                    this.current_.numval, this.current_, this.current_.baseSystem));
            } else {
                NapiLog.logError(this.lexer_ + "syntax error, except STRING or NUMBER in array");
                return nullptr;
            }

            if (arrayType == 0) {
                arrayType = this.current_.type;
            } else if (arrayType != this.current_.type) {
                NapiLog.logError(this.lexer_ + "syntax error, not allow mix type array");
                return null;
            }

            if (this.lexer_.lex(this.current_)) {
                if (this.current_.type == code(']')) {
                    break;
                } else if (this.current_.type != code(',')) {
                    NapiLog.logError(this.lexer_ + "syntax error, except ',' or ']'");
                    return null;
                }
            } else return null;
        }
        if (this.current_.type != code(']')) {
            NapiLog.logError(this.lexer_ + "syntax error, miss ']' at end of array");
            return null;
        }
        return array;
    }
    parseTerm(name) {
        if (!this.lexer_.lex(this.current_)) {
            NapiLog.logError("syntax error, miss value of config term");
            return null;
        }
        let term = new ConfigTerm(name, null)
        switch (this.current_.type) {
            case TokenType.BOOL:
                term.addChild(new AstObject("", ObjectType.PARSEROP_BOOL, this.current_.numval, this.current_));
                break;
            case TokenType.STRING:
                term.addChild(new AstObject("", ObjectType.PARSEROP_STRING, this.current_.strval, this.current_));
                break;
            case TokenType.NUMBER:
                term.addChild(new AstObject("", ObjectType.PARSEROP_UINT64,
                    this.current_.numval, this.current_, this.current_.baseSystem));
                break;
            case code('['): {
                let list = this.parseArray();
                if (list == null) {
                    return null;
                } else {
                    term.addChild(list);
                }
                break;
            }
            case code('&'):
                if (!this.lexer_.lex(this.current_) ||
                    (this.current_.type != TokenType.LITERAL && this.current_.type != TokenType.REF_PATH)) {
                    NapiLog.logError("syntax error, invalid config term definition");
                    return null;
                }
                term.addChild(new AstObject("", ObjectType.PARSEROP_NODEREF, this.current_.strval, this.current_));
                break;
            case TokenType.DELETE:
                term.addChild(new AstObject("", ObjectType.PARSEROP_DELETE, this.current_.strval, this.current_));
                break;
            default:
                NapiLog.logError("syntax error, invalid config term definition");
                return null;
        }

        if (!this.lexer_.lex(this.current_) || this.current_.type != code(';')) {
            NapiLog.logError("syntax error, miss ';'");
            return null;
        }

        return term;
    }
    parseTemplate() {
        if (!this.lexer_.lex(this.current_) || this.current_.type != TokenType.LITERAL) {
            NapiLog.logError("syntax error, template miss name");
            return null;
        }
        let name = copy(this.current_);
        let node = this.parseNode(name, false);
        if (node == null) {
            return node;
        }

        node.setNodeType(NodeRefType.NODE_TEMPLATE);
        return node;
    }

    parseNodeRef(name) {
        if (!this.lexer_.lex(this.current_) ||
            (this.current_.type != TokenType.LITERAL && this.current_.type != TokenType.REF_PATH)) {
            NapiLog.logError(this.lexer_ + "syntax error, miss node reference path");
            return nullptr;
        }
        let refPath = this.current_.strval;
        let node = this.parseNode(name);
        if (node == null) {
            return null;
        }

        let configNode = node;
        configNode.setNodeType(NodeRefType.NODE_REF);
        configNode.setRefPath(refPath);
        return node;
    }
    parseNodeDelete(name) {
        let node = this.parseNode(name);
        if (node == null) {
            return null;
        }

        node.setNodeType(NodeRefType.NODE_DELETE);
        return node;
    }
    parseNodeCopy(name) {
        let nodePath = this.current_.strval;

        let node = this.parseNode(name);
        if (node == null) {
            return null;
        }

        let nodeCopy = node;
        nodeCopy.setNodeType(NodeRefType.NODE_COPY);
        nodeCopy.setRefPath(nodePath);

        return node;
    }
    parseNodeWithRef(name) {
        switch (this.current_.type) {
            case TokenType.REF_PATH:
            case TokenType.LITERAL:
                return this.parseNodeCopy(name);
            case code('&'):
                return this.parseNodeRef(name);
            case TokenType.DELETE:
                return this.parseNodeDelete(name);
            case code(':'):
                return this.parseNodeInherit(name);
            default:
                NapiLog.logError("syntax error, unknown node type");
                break;
        }

        return null;
    }

    parseNodeInherit(name) {
        if (!this.lexer_.lex(this.current_) ||
            (this.current_.type != TokenType.LITERAL && this.current_.type != TokenType.REF_PATH)) {
            NapiLog.logError("syntax error, miss node inherit path");
            return null;
        }

        let inheritPath = this.current_.strval;

        let node = this.parseNode(name);
        if (node == null) {
            return null;
        }

        node.setNodeType(NodeRefType.NODE_INHERIT);
        node.setRefPath(inheritPath);
        return node;
    }
}

module.exports = {
    Parser
}