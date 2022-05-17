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
const { NapiLog } = require("./NapiLog");
const { NodeTools, DataType, NodeType } = require("./NodeTools");
const re = require("./re")

class ModifyNode {
}

function getParent(node, dest, parent) {
    return dest.parent_;
}

function isNameRepeat(node, name) {
    for (let i in node.value_) {
        if (node.value_[i].name_ == name) return true;
    }
    return false;
}

ModifyNode.modifyName = function (files, root, node, name) {
    let parent = getParent(root, node, null);
    if (parent == null) {
        NapiLog.logError("不能改变root节点名称");
        return false;
    }
    node.name_ = name;
    return true;
}

ModifyNode.modifyNodeType = function (files, root, node, type) {//节点类型改变
    let parent = getParent(root, node, null);
    if (parent == null) {
        NapiLog.logError("不能改变root节点类型");
        return false;
    }
    switch (type) {
        case NodeType.COPY:
        case NodeType.INHERIT:
        case NodeType.REFERENCE:
            node.ref_ = "_unknow_";
            break;
        case NodeType.DELETE:
            break;
        case NodeType.TEMPLETE:
            break;
    }
    node.nodeType_ = type;
    return true;
}

ModifyNode.addChildAttr = function (root, node) {
    //添加子属性，默认为int8
    for (let i = 1; i < 256; i++) {
        let name = "attr_" + i;
        if (!isNameRepeat(node, name)) {
            let newAttr = NodeTools.createNewNode(DataType.ATTR, name, NodeTools.createNewNode(DataType.INT8, "", 0))
            node.value_.push(newAttr);
            newAttr.parent_ = node;
            newAttr.value_.parent_ = newAttr;
            break;
        }
    }
}

ModifyNode.addChildNode = function (root, node) {
    //添加子节点，默认为数据类节点(不继承)
    for (let i = 1; i < 256; i++) {
        let name = "node_" + i;
        if (!isNameRepeat(node, name)) {
            let newNode = NodeTools.createNewNode(DataType.NODE, name, [], NodeType.DATA);
            node.value_.push(newNode);
            newNode.parent_ = node;
            break;
        }
    }
}

ModifyNode.deleteNode = function (node) {
    //删除子节点
    let parent = node.parent_;
    if (parent == null) {
        NapiLog.logError("不能删除root节点");
        return false;
    }
    for (let i in parent.value_) {
        if (parent.value_[i] == node) {
            parent.value_.splice(i, 1);
            return;
        }
    }
}

ModifyNode.getInheritList = function (root, node) {
    //获取可继承列表（模板列表）
    let ret = []
    let parent = getParent(root, node, null);
    if (parent == null) return ret;
    for (let i in parent.value_) {
        //获取同级模板节点
        let pn = parent.value_[i]
        if (pn.type_ == DataType.NODE && pn.nodeType_ == NodeType.TEMPLETE) {
            ret.push(pn);
        }
    }
    let ps = [parent]
    while (true) {
        let pp = getParent(root, parent, null);
        if (pp == null) break;
        ps.splice(0, 0, pp)
    }
    let ptemp = null;
    for (let i in ps) {
        if (ps[i].nodeType_ == NodeType.INHERIT) {
            ptemp = NodeTools.findChildByName(ptemp, ps[i].ref_)
        }
        else if (ptemp == null) {
            ptemp = ps[i]
        }
        else {
            ptemp = NodeTools.findChildByName(ptemp, ps[i].name_)
        }
        if (ptemp == null) break;
    }
    if (ptemp != null && ptemp != parent) {
        for (let i in ptemp.value_) {
            //获取其他同级模板节点
            let pn = ptemp.value_[i]
            if (pn.type_ == DataType.NODE && pn.nodeType_ == NodeType.TEMPLETE) {
                ret.push(pn);
            }
        }
    }

    return ret;
}
ModifyNode.getInheritNameList = function (root, node) {
    let nodes = ModifyNode.getInheritList(root, node);
    let ret = []
    for (i in nodes) {
        ret.push(nodes[i].name_)
    }
    return ret;
}
ModifyNode.getInheritTemplete = function (root, node) {
    let parent = getParent(root, node, null);
    if (parent == null)
        return null;
    let ilist = ModifyNode.getInheritList(root, node);
    for (let i in ilist) {
        if (parent.nodeType_ == NodeType.TEMPLETE) {
            if (parent.ref_ == ilist[i].name_)
                return ilist[i];
        }
        else if (parent.name_ == ilist[i].name_) {
            return ilist[i]
        }
    }
    return null;
}

module.exports = {
    ModifyNode
}