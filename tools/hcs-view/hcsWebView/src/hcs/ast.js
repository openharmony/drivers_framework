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
const { NapiLog } = require("./NapiLog")
const { TokenType } = require("./lexer")

var ObjectType = {
    PARSEROP_UINT8: 0x01,
    PARSEROP_UINT16: 2,
    PARSEROP_UINT32: 3,
    PARSEROP_UINT64: 4,
    PARSEROP_STRING: 5,
    PARSEROP_CONFNODE: 6,
    PARSEROP_CONFTERM: 7,
    PARSEROP_ARRAY: 8,
    PARSEROP_NODEREF: 9,
    PARSEROP_DELETE: 10,
    PARSEROP_BOOL: 11,
};

var NodeRefType = {
    NODE_NOREF: 0,
    NODE_COPY: 1,
    NODE_REF: 2,
    NODE_DELETE: 3,
    NODE_TEMPLATE: 4,
    NODE_INHERIT: 5,
};

var HcsErrorNo = {
    NOERR: 0,  /* No error */
    EFAIL: 1,      /* Process fail */
    EOOM: 2,       /* Out of memory */
    EOPTION: 3,    /* Option error */
    EREOPENF: 4,   /* Reopen argument */
    EINVALF: 5,    /* Invalid file */
    EINVALARG: 6,  /* Invalid argument */
    EDECOMP: 7,    /* Decompile error */
    EOUTPUT: 8,    /* Output error */
    EASTWALKBREAK: 9,  /* Break ast walk */
};

var UINT8_MAX = 255;
var UINT16_MAX = 65535;
var UINT32_MAX = 0xffffffff;  /* 4294967295U */
var UINT64_MAX = 0xffffffffffffffff; /* 18446744073709551615ULL */

class AstObject {
    constructor(name, type, value, bindToken, jinzhi) {
        let callType = Object.prototype.toString.call(name);
        if (callType == "[object Object]") {
            this.constructorSis(name.name_, name.type_, name.stringValue_);
            this.integerValue_ = name.integerValue_;            
        } else if (callType == "[object String]") {
            if (Object.prototype.toString.call(value) == "[object Number]") {
                if (Object.prototype.toString.call(bindToken) == "[object Object]") {
                    this.constructorSiit(name, type, value, bindToken, jinzhi)
                }
                else {
                    this.constructorSii(name, type, value, jinzhi)
                }
            }
            else if (Object.prototype.toString.call(value) == "[object String]") {
                if (Object.prototype.toString.call(bindToken) == "[object Object]") {
                    this.constructorSist(name, type, value, bindToken)
                }
                else {
                    this.constructorSis(name, type, value)
                }
            }
            else {
                NapiLog.logError("err1");
            }
        } else {
            NapiLog.logError("err2")
        }        
    }

    constructorSii(name, type, value, jinzhi) {
        this.type_ = type
        this.name_ = name
        this.parent_ = null;
        this.lineno_ = 0
        this.opCode_ = 0
        this.size_ = 0
        this.subSize_ = 0
        this.hash_ = 0
        this.integerValue_ = value
        this.jinzhi_ = jinzhi
    }

    constructorSis(name, type, value) {
        this.constructorSii(name, type, 0, 10)
        this.stringValue_ = value
    }

    constructorSiit(name, type, value, bindToken, jinzhi) {
        this.constructorSii(name, type, value, jinzhi)
        this.lineno_ = bindToken.lineNo;
        this.src_ = bindToken.src;

        switch (type) {
            case ObjectType.PARSEROP_UINT8:  /* fall-through */
            case ObjectType.PARSEROP_UINT16: /* fall-through */
            case ObjectType.PARSEROP_UINT32: /* fall-through */
            case ObjectType.PARSEROP_UINT64:
                this.type_ = this.fitIntegerValueType(value);
                break;
            default:
                break;
        }
    }

    constructorSist(name, type, value, bindToken) {
        this.constructorSiit(name, type, 0, bindToken, 10);
        this.stringValue_ = value;
    }

    fitIntegerValueType(value) {
        if (value <= UINT8_MAX) {
            return ObjectType.PARSEROP_UINT8;
        } else if (value <= UINT16_MAX) {
            return ObjectType.PARSEROP_UINT16;
        } else if (value <= UINT32_MAX) {
            return ObjectType.PARSEROP_UINT32;
        } else {
            return ObjectType.PARSEROP_UINT64;
        }
    }
    addChild(childObj) {
        if (childObj == null) {
            return false;
        }
        if (this.child_ == null) {
            this.child_ = childObj;
            let childNext = childObj;
            while (childNext != null) {
                childNext.parent_ = this;
                childNext = childNext.next_;
            }
        } else {
            return this.child_.addPeer(childObj);
        }

        return true;
    }

    addPeer(peerObject) {
        if (peerObject == null) {
            return false;
        }

        if (this == peerObject) {
            NapiLog.logError("add self as peer");
            return false;
        }

        if (this.next_ == null) {
            this.next_ = peerObject;
        } else {
            let lastNode = this.next_;
            while (lastNode.next_ != null) {
                lastNode = lastNode.next_;
            }
            lastNode.next_ = peerObject;
        }

        let peer = peerObject;
        while (peer) {
            peer.parent_ = this.parent_;
            peer = peer.next_;
        }

        return true;
    }

    // operate<<
    merge(srcObj) {
        if (srcObj.name_ != this.name_) {
            NapiLog.logError(this.sourceInfo() + "merge different node to" + srcObj.sourceInfo());
            return false;
        }

        if (srcObj.type_ != this.type_) {
            NapiLog.logError(this.sourceInfo() + "conflict type with " + srcObj.sourceInfo());
            return false;
        }

        this.src_ = srcObj.src_;
        this.lineno_ = srcObj.lineno_;
        this.stringValue_ = srcObj.stringValue_;
        this.integerValue_ = srcObj.integerValue_;

        return true;
    }
    copy(src, overwrite) {
        if (src == null) {
            return false;
        }

        if (overwrite) {
            this.src_ = src.src_;
            this.lineno_ = src.lineno_;
            this.integerValue_ = src.integerValue_;
            this.stringValue_ = src.stringValue_;
        }

        return true;
    }

    move(src) {
        if (!this.copy(src, true)) {
            return false;
        }
        src.separate();
        return true;
    }

    sourceInfo() {
        return this.src_ + ":" + this.lineno_ + " ";
    }

    remove() {
        this.separate();
        this.child_ = null;
        this.next_ = null;
    }

    lookup(name, type) {
        let peer = this.child_;
        while (peer != null) {
            if (peer.name_ == name && (type == 0 || peer.type_ == type)) {
                return peer;
            }

            peer = peer.next_;
        }

        return null;
    }
    
    isNumber() {
        return this.type_ >= ObjectType.PARSEROP_UINT8 && type_ <= ObjectType.PARSEROP_UINT64;
    }

    isNode() {
        return this.type_ == ObjectType.PARSEROP_CONFNODE;
    }

    isTerm() {
        return this.type_ == ObjectType.PARSEROP_CONFTERM;
    }
    isArray() {
        return this.type_ == ObjectType.PARSEROP_ARRAY;
    }
    separate() {
        if (this.parent_ == null) {
            return;
        }
        if (this.parent_.child_ == this) {
            this.parent_.child_ = this.next_;
            this.next_ = null;
            return;
        }

        let pre = this.parent_.child_;
        while (pre != null) {
            if (pre.next_ == this) {
                let tmp = this.next_;
                this.next_ = null;
                pre.next_ = tmp;
                break;
            }

            pre = pre.next_;
        }
    }
    setParent(parent) { 
        this.parent_ = parent; 
    }
    SetSize(size) { 
        this.size_ = size; 
    }
    SetSubSize(size) { 
        this.subSize_ = size; 
    }
    GetSubSize() { 
        return this.subSize_; 
    }
    SetHash(hash) { 
        this.hash_ = hash; 
    }
    GetSize() { 
        return this.size_; 
    }
    GetHash() { 
        return this.hash_; 
    }
    next() { 
        return this.next_; 
    }
    child() { 
        return this.child_; 
    }
    name() { 
        return this.name_; 
    }
    stringValue() { 
        return this.stringValue_; 
    }
    IntegerValue() { 
        return this.integerValue_; 
    }
    type() { 
        return this.type_; 
    }
    OpCode() { 
        return this.opCode_; 
    }
    SetOpCode(opcode) { 
        this.opCode_ = opcode; 
    }
    hasDuplicateChild() { 
        return false; 
    }
    isElders(child) {
        let p = child;
        while (p != null) {
            if (p == this) {
                return true;
            }
            p = p.parent_;
        }
        return false;
    }
    parent() {
        return this.parent_;
    }
}

class ConfigNode extends AstObject {
    constructor(name, nodeType, refName) {
        if (Object.prototype.toString.call(name) == "[object String]") {
            super(name, ObjectType.PARSEROP_CONFNODE, "");
            this.refNodePath_ = refName;
            this.nodeType_ = nodeType;
            this.inheritIndex_ = 0;
            this.inheritCount_ = 0;
            this.templateSignNum_ = 0;
        }
        else if (Object.prototype.toString.call(nodeType) == "[object Number]") {
            super(name.strval, ObjectType.PARSEROP_CONFNODE, 0, name);
            this.refNodePath_ = refName;
            this.nodeType_ = nodeType;
            this.inheritIndex_ = 0;
            this.inheritCount_ = 0;
            this.templateSignNum_ = 0;
        }
        else {
            super(name, ObjectType.PARSEROP_CONFNODE, "");
            this.refNodePath_ = refName;
            this.nodeType_ = nodeType;
            this.inheritIndex_ = 0;
            this.inheritCount_ = 0;
            this.templateSignNum_ = 0;

            let child = name.child_;
            while (child != null) {
                super.addChild(AstObjectFactory.build(child));
                child = child.next();
            }
        }
        this.subClasses_ = [];
    }
    NodeTypeToStr(type) {
        let type2StringMap = {
            0: "",
            1: "NodeCopy",
            2: "NodeReference",
            3: "NodeDelete",
            4: "NodeInherit",
            5: "NodeTemplate",
        };
        return type2StringMap[type];
    }

    // operator<<
    castFrom(astObject) {
        return astObject;
    }
    getNodeType() {
        return this.nodeType_;
    }
    getRefPath() {
        return this.refNodePath_;
    }
    merge(srcObj) {
        if (srcObj == null) {
            return true;
        }
        if (!srcObj.isNode() || srcObj.name() != this.name_) {
            NapiLog.logError( sourceInfo() + "merge conflict type with " + srcObj.sourceInfo());
            return false;
        }

        let srcNode = srcObj;
        if (srcNode.getNodeType() == TokenType.DELETE) {
            srcObj.separate();
            this.separate();
            return true;
        }

        this.nodeType_ = srcNode.nodeType_;
        this.refNodePath_ = srcNode.refNodePath_;

        let childSrc = srcObj.child();
        while (childSrc != null) {
            let childSrcNext = childSrc.next();
            let childDst = this.lookup(childSrc.name(), childSrc.type());
            if (childDst == null) {
                childSrc.separate();
                this.addChild(childSrc);
            } else if (!childDst.merge(childSrc)){
                return false;
            }
            childSrc = childSrcNext;
        }
        return true;
    }
    setNodeType(nodeType) {
        this.nodeType_ = nodeType;
    }
    setRefPath(ref) {
        this.refNodePath_ = ref;
    }
    hasDuplicateChild() {
        let symMap = {};
        let child = this.child_;
        while (child != null) {
            let sym = symMap[child.name()];
            if (sym != undefined) {
                NapiLog.logError(child.sourceInfo() + "redefined, first definition at " + sym.second.sourceInfo());
                return true;
            }
            symMap[child.name()] = child;
            child = child.next();
        }

        return false;
    }
    inheritExpand(refObj) {
        if (refObj == null) {
            NapiLog.logError(sourceInfo() + "inherit invalid node: " + refNodePath_);
            return false;
        }

        if (!this.copy(refObj, false)) {
            return false;
        }

        let templateNode = this.castFrom(refObj);
        if (!this.compare(templateNode)) {
            return false;
        }
        this.inheritIndex_ = templateNode.inheritCount_++;
        templateNode.subClasses_.push(this);
        return true;
    }
    refExpand(refObj) {
        if (this.nodeType_ == NodeRefType.NODE_DELETE) {
            this.separate();
            return true;
        }

        if (refObj.isElders(this)) {
            NapiLog.logError(sourceInfo() + "circular reference " + refObj.sourceInfo());
            return false;
        }

        let ret = true;
        if (this.nodeType_ == NodeRefType.NODE_REF) {
            ret = this.nodeRefExpand(refObj);
        } else if (nodeType_ == NodeRefType.NODE_COPY) {
            ret = nodeCopyExpand(refObj);
        }

        return ret;
    }
    copy(src, overwrite) {
        let child = src.child();
        while (child != null) {
            let dst = this.lookup(child.name(), child.type());
            if (dst == null) {
                this.addChild(AstObjectFactory.build(child));
            } else if (!dst.copy(child, overwrite)){
                return false;
            }
            child = child.next();
        }

        return true;
    }
    move(src) {
        return super.move(src);
    }
    nodeRefExpand(ref) {
        if (ref == null) {
            NapiLog.logError(sourceInfo() + "reference node '" + refNodePath_ + "' not exist");
            return false;
        }
        return ref.move(this);
    }
    nodeCopyExpand(ref) {
        if (ref == null) {
            NapiLog.logError(sourceInfo() + "copy node '" + refNodePath_ + "' not exist");
            return false;
        }
        this.nodeType_ = NodeRefType.NODE_NOREF;
        return this.copy(ref, false);
    }
    compare(other) {
        let objChild = this.child_;
        while (objChild != null) {
            let baseObj = this.lookup(objChild.name(), objChild.type());
            if (baseObj == null) {
                NapiLog.logError(objChild.sourceInfo() + "not in template node: " + other.sourceInfo());
                return false;
            }
            if (objChild.isNode()) {
                return this.castFrom(objChild).compare(this.castFrom(baseObj));
            }

            objChild = objChild.next();
        }
        return true;
    }
    InheritIndex() {
        return this.inheritIndex_;
    }
    InheritCount() {
        return this.inheritCount_;
    }
    TemplateSignNum() {
        return this.templateSignNum_;
    }
    SetTemplateSignNum(sigNum) {
        this.templateSignNum_ = sigNum;
    }
    SubClasses() {
        return this.subClasses_;
    }
}

class ConfigTerm extends AstObject {
    constructor(name, value) {
        if (Object.prototype.toString.call(name) == "[object String]") {
            super(name, ObjectType.PARSEROP_CONFTERM, 0);
            this.signNum_ = 0;
            this.child_ = value;
            if (value != null) value.this.setParent(this);
        }
        else if (Object.prototype.toString.call(value) == "[object Object]" ||
          Object.prototype.toString.call(value) == "[object Null]") {
            super(name.strval, ObjectType.PARSEROP_CONFTERM, 0, name);
            this.signNum_ = 0;
            this.child_ = value;
            if (value != null) value.this.setParent(this);
        }
        else {
            super(name.name_, ObjectType.PARSEROP_CONFTERM, 0);
            this.signNum_ = 0;
            this.child_ = value;
            if (value != null) value.this.setParent(this);
            super.addChild(AstObjectFactory.build(name.child_));
        }
    }

    // operator<<
    castFrom(astObject) {
        return astObject;
    }
    merge(srcObj) {
        if (!srcObj.isTerm()) {
            NapiLog.logError(sourceInfo() + "merge conflict type with " + srcObj.sourceInfo());
            return false;
        }

        let value = srcObj.child();
        srcObj.child().separate();
        this.child_ = null;
        this.addChild(value);
        return true;
    }
    
    refExpand(refObj) {
        if (child_.type() == ObjectType.PARSEROP_DELETE) {
            this.separate();
            return true;
        }

        if (child_.type() != ObjectType.PARSEROP_NODEREF) {
            return true;
        }

        if (refObj == null || !refObj.isNode() ||
            ConfigNode.castFrom(refObj).getNodeType() == NodeRefType.NODE_REF ||
            ConfigNode.castFrom(refObj).getNodeType() == NodeRefType.NODE_TEMPLATE ||
            ConfigNode.castFrom(refObj).getNodeType() == NodeRefType.NODE_DELETE) {
            NapiLog.logError(sourceInfo() + "reference invalid node '" + child_.stringValue() + '\'');
            return false;
        }

        this.refNode_ = refObj;
        return true;
    }
    copy(src, overwrite) {
        if (!overwrite) {
            return true;
        }
        if (this.child_.type() != src.child().type() && (!this.child_.isNumber() || !src.child().isNumber())) {
            NapiLog.logError(src.sourceInfo() + "overwrite different type with:" + child_.sourceInfo());
            return false;
        }
        return this.child_.copy(src.child(), overwrite);
    }
    move(src) {
        return this.child_.move(src.child());
    }
    RefNode() {
        return this.refNode_;
    }
    SetSigNum(sigNum) {
        this.signNum_ = sigNum;
    }
    SigNum() {
        return this.signNum_;
    }
}
class ConfigArray extends AstObject {
    constructor(array) {
        if (Object.prototype.toString.call(array) == "[object Object]") {
            super("", ObjectType.PARSEROP_ARRAY, 0, array); // bindToken
            this.arrayType_ = 0;
            this.arraySize_ = 0;
            if (array.type == undefined) {
                let child = array.child_;
                while (child != null) {
                    super.addChild(AstObjectFactory.build(child));
                    child = child.next();
                }
                this.arraySize_ = array.arraySize_;
                this.arrayType_ = array.arrayType_;
            }
        }
        else {
            super("", ObjectType.PARSEROP_ARRAY, 0);
            this.arrayType_ = 0;
            this.arraySize_ = 0;
        }

    }

    addChild(childObj) {
        if (super.addChild(childObj)) {
            this.arraySize_++;
            this.arrayType_ = this.arrayType_ > childObj.type() ? this.arrayType_ : childObj.type();
            return true;
        } else {
            return false;
        }
    }

    merge(srcObj) {
        if (!srcObj.isArray()) {
            NapiLog.logError(sourceInfo() + "merge conflict type with " + srcObj.sourceInfo());
            return false;
        }

        let value = srcObj.child();
        value.separate();
        this.child_ = value;
        return true;
    }

    copy(src, overwrite) {
        if (!overwrite) {
            return true;
        }
        let array = ConfigArray.castFrom(src);
        this.child_ = null;
        let t = array.child_;
        while (t != null) {
            addChild(AstObjectFactory.build(t));
        }
        return true;
    }

    castFrom(astObject) {
        return astObject;
    }

    arraySize() {
        return this.arraySize_;
    }

    arrayType() {
        return this.arrayType_;
    }
}

class AstObjectFactory { }
AstObjectFactory.build = function (object) {
    switch (object.type()) {
        case ObjectType.PARSEROP_CONFNODE:
            return new ConfigNode(object);
        case ObjectType.PARSEROP_CONFTERM:
            return new ConfigTerm(object);
        case ObjectType.PARSEROP_ARRAY:
            return new ConfigArray(object);
        default:
            return new AstObject(object);
    }
}

class Ast {
    constructor(astRoot) {
        this.astRoot_ = astRoot;
        this.redefineChecked_ = false;
    }
    setw(l) {
        let ret = "";
        for (let i = 0; i < l; i++)ret += " ";
        return ret;
    }

    dump(prefix) {
        NapiLog.logError("dump ", prefix, " AST:");
        this.walkForward(this.astRoot_, (current, walkDepth) => {
            switch (current.type_) {
                case ObjectType.PARSEROP_UINT8:
                case ObjectType.PARSEROP_UINT16:
                case ObjectType.PARSEROP_UINT32:
                case ObjectType.PARSEROP_UINT64:
                    NapiLog.logInfo(this.setw(walkDepth * 4) + "[" + current.integerValue_ + "]"); break;
                case ObjectType.PARSEROP_STRING:// 5
                    NapiLog.logInfo(this.setw(walkDepth * 4) + current.stringValue_); break;
                case ObjectType.PARSEROP_CONFNODE:// 6 content
                    if (current.name_ == "blockSize") {
                        current.name_ = "blockSize"
                    }
                    NapiLog.logInfo(this.setw(walkDepth * 4) + current.name_ + " :"); break;
                case ObjectType.PARSEROP_CONFTERM:// 7 Attribute name
                    if (current.name_ == "funcNum") {
                        current.name_ = "funcNum"
                    }
                    NapiLog.logInfo(this.setw(walkDepth * 4) + current.name_ + " = "); break;
                case ObjectType.PARSEROP_ARRAY:
                    NapiLog.logInfo(this.setw(walkDepth * 4) + current.name_); break;
                case ObjectType.PARSEROP_NODEREF:
                    NapiLog.logInfo(this.setw(walkDepth * 4) + current.name_); break;
                case ObjectType.PARSEROP_DELETE:
                    NapiLog.logInfo(this.setw(walkDepth * 4) + current.name_); break;
            }
            return HcsErrorNo.NOERR;
        });
    }

    getAstRoot() {
        return this.astRoot_;
    }

    merge(astList) {
        if (!this.redefineCheck()) {
            return false;
        }
        for (let i = 0; i < astList.length; i++) {
            let astIt = astList[i];
            if (!astIt.redefineCheck()) {
                return false;
            }
            if (this.astRoot_ != null && !this.astRoot_.merge(astIt.astRoot_)) {
                return false;
            } else if (this.astRoot_ == null) {
                this.astRoot_ = astIt.getAstRoot();
            }
        }
        return true;
    }

    getchild(node, name) {
        let p = node.child_
        while (p != null) {
            if (p.name_ == name) return p;
            p = p.next_;
        }
        return null;
    }

    getpath(node) {
        NapiLog.logError("----path start----")
        let p = node
        while (p != null) {
            NapiLog.logError(p.name_)
            p = p.parent_
        }
        NapiLog.logError("----path end----")
    }

    expand() {
        let n1, n2;

        if (!this.redefineCheck()) {
            return false;
        }

        if (this.astRoot_.lookup("module", ObjectType.PARSEROP_CONFTERM) == null) {
            NapiLog.logError(astRoot_.sourceInfo() + "miss 'module' attribute under root node");
            return false;
        }

        if (!this.nodeExpand()) {
            return false;
        }

        if (!this.inheritExpand()) {
            return false;
        }
        this.dump("expanded");
        return true;
    }

    nodeExpand() {
        return this.walkBackward(this.astRoot_, (current, walkDepth) => {
            if (current.isNode()) {
                let node = current;
                if (node.getNodeType() == NodeRefType.NODE_DELETE) {
                    current.remove();child_
                    return HcsErrorNo.NOERR;
                }
                if (node.getNodeType() != NodeRefType.NODE_REF && node.getNodeType() != NodeRefType.NODE_COPY) {
                    return HcsErrorNo.NOERR;
                }

                // current node maybe deleted after reference expand, never use it after this
                let ret = node.refExpand(this.lookup(current, node.getRefPath()));
                if (!ret) {
                    return HcsErrorNo.EFAIL;
                }
            } else if (current.isTerm()) {
                let ref;
                if (current.child_.type() == ObjectType.PARSEROP_DELETE) {
                    current.remove();
                    return HcsErrorNo.NOERR;
                }
                if (current.child_.type() == ObjectType.PARSEROP_NODEREF) {
                    ref = lookup(current, current.child_.stringValue());
                    if (!current.refExpand(ref)) {
                        return HcsErrorNo.EFAIL;
                    }
                }
            }
            return HcsErrorNo.NOERR;
        });
    }

    walkBackward(startObject, callback) {
        let backWalkObj = startObject;
        let next = null;
        let parent = null;
        let walkDepth = 0;
        let preVisited = false;

        while (backWalkObj != null) {
            let backWalk = true;
            if (backWalkObj.child_ == null || preVisited) {
                next = backWalkObj.next_;
                parent = backWalkObj.parent();

                /* can safe delete current in callback */
                if (callback(backWalkObj, walkDepth) != HcsErrorNo.NOERR) {
                    return false;
                }
            } else if (backWalkObj.child_) {
                walkDepth++;
                backWalkObj = backWalkObj.child_;
                backWalk = false;                   
            }
            if (backWalk) {
                if (backWalkObj == startObject) {
                    break;
                }
    
                if (next != null) {
                    backWalkObj = next;
                    preVisited = false;
                } else {
                    backWalkObj = parent;
                    preVisited = true;
                    walkDepth--;
                }            
            }
        }
        return true;
    }

    inheritExpand() {
        return this.walkForward(this.astRoot_, (current, ii) => {
            if (current.isNode()) {
                let node = current;
                if (node.getNodeType() != NodeRefType.NODE_INHERIT) {
                    return HcsErrorNo.NOERR;
                }
                let inherit = this.lookup(current, node.getRefPath());
                if (!node.inheritExpand(inherit)) {
                    return HcsErrorNo.EFAIL;
                }
            }

            return HcsErrorNo.NOERR;
        });
    }

    redefineCheck() {
        if (this.redefineChecked_) {
            return true;
        }

        let ret = this.walkForward(this.astRoot_, (current, ii) => {
            if (current.isNode() && current.hasDuplicateChild()) {
                return HcsErrorNo.EFAIL;
            }

            return HcsErrorNo.NOERR;
        });

        this.redefineChecked_ = true;
        return ret;
    }

    walkForward(startObject, callback) {
        let forwardWalkObj = startObject;
        let walkDepth = 0;
        let preVisited = false;

        while (forwardWalkObj != null) {
            let forward = true;
            if (!preVisited) {
                let ret = callback(forwardWalkObj, walkDepth);
                if (ret && ret != HcsErrorNo.EASTWALKBREAK) {
                    return false;
                } else if (ret != HcsErrorNo.EASTWALKBREAK && forwardWalkObj.child_ != null) {

                    /* when callback return HcsErrorNo.EASTWALKBREAK, not walk current's child */
                    walkDepth++;
                    forwardWalkObj = forwardWalkObj.child_;
                    forward = false;                    
                }
            }

            if (forward) {
                if (forwardWalkObj == startObject) {
                    break;
                }
    
                if (forwardWalkObj.next_ != null) {
                    forwardWalkObj = forwardWalkObj.next_;
                    preVisited = false;
                } else {
                    forwardWalkObj = forwardWalkObj.parent();
                    preVisited = true;
                    walkDepth--;
                }
            }

        }

        return true;
    }

    lookup(startObj, path) {
        if (path.indexOf('.') < 0) {
            return startObj.parent_.lookup(path, 0);
        }

        let splitPath = this.splitNodePath(path, '.');
    }
    splitNodePath(path, separator) {
        let splitList = path.split(separator)
        return splitList;
    }
}

module.exports = {
    AstObject,
    ConfigNode,
    ConfigTerm,
    ConfigArray,
    NodeRefType,
    ObjectType,
    Ast
}