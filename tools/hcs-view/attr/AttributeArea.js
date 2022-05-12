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
class AttributeArea {
    constructor() {
        document.attrCallback = this;
        this.area = document.getElementById("attribute_area");
        this.htmlStr = ""
        this.callbackFunc = null;
        this.freshInputValue_ = []
    }
    clear() {
        this.htmlStr = ""
    }
    flush() {
        this.area.innerHTML = this.htmlStr
        while (this.freshInputValue_.length > 0) {
            let v = this.freshInputValue_.shift();
            let e = document.getElementById(v[0]);
            e.value = v[1];
        }
    }
    addTitle(name) {
        this.htmlStr += '<p class="att_title">' + name + '</p>';
    }
    addInput(searchId, label, default_, disable) {
        let ret = '<label class="input_text_readonly">' + label + '</label>'
        ret += '<input id="' + searchId + '"';
        ret += ' class="input_text"';
        if (default_.indexOf('"') >= 0) {
            ret += ' value=""';
            this.freshInputValue_.push([searchId, default_])
        }
        else
            ret += ' value="' + default_ + '"';
        if (disable)
            ret += ' disabled="disabled"';
        ret += ' oninput="document.attrCallback.Event(' + "'input', " + "'" + searchId + "'" + ')" /><br>'
        this.htmlStr += ret;
    }
    addTextArea(searchId, label, default_) {
        let ret = '<label class="input_text_readonly">' + label + '</label><br>'
        ret += '<textarea id="' + searchId + '"';
        ret += ' class="text_area"';
        ret += ' oninput="document.attrCallback.Event(' + "'input', " + "'" + searchId + "'" + ')">'
        ret += default_;
        ret += '</textarea><br>';//cols="20" rows="10"
        this.htmlStr += ret;
    }
    addButton(searchId, label) {
        if (label.length > 13) label = label.substring(0, 10) + "..."
        let text = '" class="button_click" type="button" onclick="document.attrCallback.Event('
        this.htmlStr += '<button id="' + searchId + text + "'button', " +
        "'" + searchId + "'" + ')">' + label + '</button><br>';
    }
    addSelect(searchId, label, selectList, default_, disable) {
        let ret = '<label class="input_text_readonly">' + label + '</label>'
        ret += '<select id="' + searchId + '"';
        ret += ' class="select_list"'
        ret += ' size="1"'
        if (disable)
            ret += ' disabled="disabled"';
        ret += ' onchange="document.attrCallback.Event(' + "'select', " + "'" + searchId + "'" + ')">';
        for (let i in selectList) {
            if (selectList[i] == default_) {
                ret += '<option selected="selected">' + selectList[i] + "</option>";
            }
            else {
                ret += "<option>" + selectList[i] + "</option>";
            }
        }
        ret += "</select><br>";
        this.htmlStr += ret;
    }
    addGap(type) {
        if (type == 0) this.htmlStr += '<br>';
    }
    Event(type, value) {
        let cbv = "";
        if (type == "input") {
            cbv = document.getElementById(value).value
        }
        else if (type == "select") {
            cbv = document.getElementById(value).value
        }
        if (this.callbackFunc != null) {
            this.callbackFunc(value, type, cbv)
        }
    }
    registRecvCallback(func) {
        this.callbackFunc = func
    }
}
AttributeArea.pInstance_ = null;
AttributeArea.gi = function () {
    if (AttributeArea.pInstance_ == null) AttributeArea.pInstance_ = new AttributeArea();
    return AttributeArea.pInstance_;
}

module.exports = {
    AttributeArea
}