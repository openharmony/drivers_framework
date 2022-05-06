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
const { AttributeArea } = require("./attr/AttributeArea");
const { ModifyNode } = require("./hcs/ModifyNode");
const { DataType, NodeType } = require("./hcs/NodeTools");
const { NodeTools } = require("./hcs/NodeTools");

class AttrEditor {
    constructor() {
        AttributeArea.gi().registRecvCallback(this.onChange);
        this.files_ = []
        this.callback_ = null;
    }
    setFiles(files) {
        this.files_ = files;
    }
    freshEditor(fn, node) {
        this.filePoint_ = fn;
        this.node_ = node;
        if (fn in this.files_) {
            this.root_ = this.files_[fn];
        }

        AttributeArea.gi().clear();
        if (this.node_ != null) {
            AttributeArea.gi().addTitle(this.node_.name_)
            switch (this.node_.type_) {
                case 6:// Nodes
                    switch (this.node_.nodeType_) {
                        case 0:// Data class nodes, not inherit
                            this.freshDataNodeNotInheritEditor(this.node_);
                            break;
                        case 1:// Copy class nodes
                        case 2:// Reference modification class nodes
                        case 5:// Data class nodes, inherit
                            this.freshcopyNodeEditor(this.node_);
                            break;
                        case 3:// Deletion class nodes
                            this.freshdeleteNodeEditor(this.node_);
                            break;
                        case 4:// Templete Class nodes
                            this.freshTempletNodeEditor(this.node_);
                            break;
                        default:
                            break;
                    }
                    break;
                case 7:// Attribute
                    this.freshDefineAttributeEditor(this.node_);
                    break;
            }
        }
        else AttributeArea.gi().addTitle("属性编辑区")

        AttributeArea.gi().flush();
    }

    // Node0 -- Data class nodes, not inherit
    freshDataNodeNotInheritEditor(node) {
        AttributeArea.gi().addSelect("node_type", "节点类型", AttrEditor.NODE_TYPE_STR, 
          AttrEditor.NODE_TYPE_STR[node.nodeType_], this.root_ == this.node_);
        AttributeArea.gi().addGap(0);

        AttributeArea.gi().addInput("name", "名称", node.name_, this.root_ == this.node_);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("add_child_node", "添加子节点");
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("add_child_attr", "添加子属性");
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("delete", "删除");
    }
    changeDataNodeNotInherit(searchId, type, value) {
        if (searchId == "node_type") {
            ModifyNode.modifyNodeType(this.files_, this.root_, this.node_, AttrEditor.NODE_TYPE_STR.indexOf(value));
            this.freshEditor(this.filePoint_, this.node_);
        }
        if (searchId == "name" && this.node_.name_ != value) {
          ModifyNode.modifyName(this.files_, this.root_, this.node_, value);
        }
        if (searchId == "add_child_node") ModifyNode.addChildNode(this.root_, this.node_);
        if (searchId == "add_child_attr") ModifyNode.addChildAttr(this.root_, this.node_);
        if (searchId == "delete") {
            ModifyNode.deleteNode(this.node_);
            this.freshEditor();
        }
    }

    // Node1 -- Copy class nodes
    // Node2 -- Reference modification class nodes
    // Node5 -- Data class nodes, inherit
    freshcopyNodeEditor(node) {
        AttributeArea.gi().addInput("node_type", "节点类型", AttrEditor.NODE_TYPE_STR[node.nodeType_], true);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addInput("name", "名称", node.name_);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("change_target", node.ref_);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("add_child_node", "添加子节点");
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("add_child_attr", "添加子属性");
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("delete", "删除");
    }
    changecopyNode(searchId, type, value) {
        if (searchId == "name" && this.node_.name_ != value) {
          ModifyNode.modifyName(this.files_, this.root_, this.node_, value);
        }
        if (searchId == "change_target") {
            this.callCallback("change_target", this.node_);
        }
        if (searchId == "add_child_node") ModifyNode.addChildNode(this.root_, this.node_);
        if (searchId == "add_child_attr") ModifyNode.addChildAttr(this.root_, this.node_);
        if (searchId == "delete") {
            ModifyNode.deleteNode(this.node_);
            this.freshEditor();
        }
    }

    // Node3 -- Deletion class nodes
    freshdeleteNodeEditor(node) {
        AttributeArea.gi().addInput("node_type", "节点类型", AttrEditor.NODE_TYPE_STR[node.nodeType_], true);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addInput("name", "名称", node.name_);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("delete", "删除");
    }
    changedeleteNode(searchId, type, value) {
        if (searchId == "name" && this.node_.name_ != value) {
          ModifyNode.modifyName(this.files_, this.root_, this.node_, value);
        }  
        if (searchId == "delete") {
            ModifyNode.deleteNode(this.node_);
            this.freshEditor();
        }
    }

    // Node4 -- Templete Class nodes
    freshTempletNodeEditor(node) {
        AttributeArea.gi().addInput("node_type", "节点类型", AttrEditor.NODE_TYPE_STR[node.nodeType_], true);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addInput("name", "名称", node.name_);
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("add_child_node", "添加子节点");
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("add_child_attr", "添加子属性");
        AttributeArea.gi().addGap(0);
        AttributeArea.gi().addButton("delete", "删除");
    }
    changeTempletNode(searchId, type, value) {
        if (searchId == "name") {
          ModifyNode.modifyName(this.files_, this.root_, this.node_, value);
        }
        if (searchId == "add_child_node") ModifyNode.addChildNode(this.root_, this.node_);
        if (searchId == "add_child_attr") ModifyNode.addChildAttr(this.root_, this.node_);
        if (searchId == "delete") {
            ModifyNode.deleteNode(this.node_);
            this.freshEditor();
        }
    }


    // Attribute0 -- Define class properties
    freshDefineAttributeEditor(node) {
        let v = node.value_;
        AttributeArea.gi().addInput("name", "名称", node.name_);
        AttributeArea.gi().addGap(0);

        if (v.type_ == DataType.INT8 ||
            v.type_ == DataType.INT16 ||
            v.type_ == DataType.INT32 ||
            v.type_ == DataType.INT64) {
            AttributeArea.gi().addSelect("value_type", "类型", AttrEditor.ATTR_TYPE_STR, AttrEditor.ATTR_TYPE_STR[0]);
            AttributeArea.gi().addGap(0);
            AttributeArea.gi().addInput("value", "属性值", NodeTools.jinZhi_10_to_x(v.value_, v.jinzhi_));
            AttributeArea.gi().addGap(0);
        }
        else if (v.type_ == DataType.STRING) {
            AttributeArea.gi().addSelect("value_type", "类型", AttrEditor.ATTR_TYPE_STR, AttrEditor.ATTR_TYPE_STR[1]);
            AttributeArea.gi().addGap(0);
            AttributeArea.gi().addInput("value", "属性值", v.value_);
            AttributeArea.gi().addGap(0);
        }
        else if (v.type_ == DataType.ARRAY) {
            AttributeArea.gi().addSelect("value_type", "类型", AttrEditor.ATTR_TYPE_STR, AttrEditor.ATTR_TYPE_STR[2]);
            AttributeArea.gi().addGap(0);
            AttributeArea.gi().addTextArea("value", "属性值", NodeTools.arrayToString(v, 20));
            AttributeArea.gi().addGap(0);
        }
        else if (v.type_ == DataType.BOOL) {
            AttributeArea.gi().addSelect("value_type", "类型", AttrEditor.ATTR_TYPE_STR, AttrEditor.ATTR_TYPE_STR[3]);
            AttributeArea.gi().addGap(0);
            AttributeArea.gi().addSelect("value", "属性值", [true, false], v.value_ == 1);
            AttributeArea.gi().addGap(0);
        }
        else if (v.type_ == DataType.REFERENCE) {
            AttributeArea.gi().addSelect("value_type", "类型", AttrEditor.ATTR_TYPE_STR, AttrEditor.ATTR_TYPE_STR[4]);
            AttributeArea.gi().addGap(0);
            AttributeArea.gi().addButton("change_target", v.value_);
            AttributeArea.gi().addGap(0);
        }
        else if (v.type_ == DataType.DELETE) {
            AttributeArea.gi().addSelect("value_type", "类型", AttrEditor.ATTR_TYPE_STR, AttrEditor.ATTR_TYPE_STR[5]);
            AttributeArea.gi().addGap(0);
        }

        AttributeArea.gi().addButton("delete", "删除");
    }

    getNodeValue(v, value) {
        switch (AttrEditor.ATTR_TYPE_STR.indexOf(value)) {
            case 0:// int
                v.type_ = DataType.INT8;
                v.value_ = 0;
                v.jinzhi_ = 10;
                break;
            case 1:// string
                v.type_ = DataType.STRING;
                v.value_ = "";
                break;
            case 2:// array
                v.type_ = DataType.ARRAY;
                v.value_ = [];
                break;
            case 3:// bool
                v.type_ = DataType.BOOL;
                v.value_ = 1;
                break;
            case 4:// ref
                v.type_ = DataType.REFERENCE;
                v.value_ = "_unknow_";
                break;
            case 5:// delete
                v.type_ = DataType.DELETE;
                break;
        }
        this.freshEditor(this.filePoint_, this.node_);
    }

    changeDefineAttribute(searchId, type, value) {
        let v = this.node_.value_;
        if (searchId == "name" && value != this.node_.name_) {
          ModifyNode.modifyName(this.files_, this.root_, this.node_, value);
        }  
        if (searchId == "value_type") {
          getNodeValue(v, value);
        }
        if (searchId == "change_target") {
            this.callCallback("change_target", v);
        }
        if (searchId == "value") {
            if (v.type_ == DataType.INT8 ||
                v.type_ == DataType.INT16 ||
                v.type_ == DataType.INT32 ||
                v.type_ == DataType.INT64) {
                let ret = NodeTools.jinZhi_x_to_10(value)
                if (!isNaN(ret[0])) {
                    v.value_ = ret[0]
                    v.jinzhi_ = ret[1]
                }
            }
            else if (v.type_ == DataType.STRING) {
                v.value_ = value;
            }
            else if (v.type_ == DataType.ARRAY) {
                v.value_ = NodeTools.stringToArray(value);
            }
            else if (v.type_ == DataType.BOOL) {
                v.value_ = value == "true" ? 1 : 0;
            }
        }
        if (searchId == "delete") {
            ModifyNode.deleteNode(this.node_);
            this.freshEditor();
        }
    }

    // Attribute1 -- Delete class properties
    // Attribute2 -- Reference class properties
    onChange(searchId, type, value) {
        let pattr = AttrEditor.gi();
        if (pattr.node_ != null) {
            AttributeArea.gi().addTitle(pattr.node_.name_)
            switch (pattr.node_.type_) {
                case 6:// nodes
                    switch (pattr.node_.nodeType_) {
                        case 0:// Data class nodes, not inherit
                            pattr.changeDataNodeNotInherit(searchId, type, value);
                            break;
                        case 1:// Copy class nodes
                        case 2:// Reference modification class nodes
                        case 5:// Data class nodes, inherit
                            pattr.changecopyNode(searchId, type, value);
                            break;
                        case 3:// Deletion class nodes
                            pattr.changedeleteNode(searchId, type, value);
                            break;
                        case 4:// Templete Class nodes
                            pattr.changeTempletNode(searchId, type, value);
                            break;
                        default:
                            break;
                    }
                    break;
                case 7:// Attribute
                    pattr.changeDefineAttribute(searchId, type, value);
                    break;
            }
            pattr.callCallback("writefile")
        }
    }
    callCallback(type, value) {
        if (this.callback_ != null) {
            this.callback_(type, value);
        }
    }
    registCallback(func) {
        this.callback_ = func;
    }
}

AttrEditor.NODE_TYPE_STR = ["数据类不继承", "复制类", "引用修改类", "删除类", "模板类", "数据类继承"];
AttrEditor.ATTR_CLASS = ["定义类属性", "删除类属性", "引用类属性"];

AttrEditor.ATTR_TYPE_STR = ["整数", "字符串", "数组", "布尔", "引用", "删除"]

AttrEditor.pInstance_ = null;
AttrEditor.gi = function () {
    if (AttrEditor.pInstance_ == null) AttrEditor.pInstance_ = new AttrEditor();
    return AttrEditor.pInstance_;
}

module.exports = {
    AttrEditor
}
