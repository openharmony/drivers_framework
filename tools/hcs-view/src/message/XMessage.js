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
const { MockMessage } = require("./mock")
const { NapiLog } = require("./../hcs/NapiLog");

class XMessage {
    constructor() {
        this.messageType = XMessage.TYPE_NONE
        try {
            this.vscode = acquireVsCodeApi();
            window.addEventListener('message', this.onRecv);
            this.messageType = XMessage.TYPE_VSCODE
        } catch (err) {
            NapiLog.logError("Error:"+err);
        }
        this.callbackFunc = null
        NapiLog.logInfo("Init XMessage : " + this.messageType)
    }
    registRecvCallback(func) {
        this.callbackFunc = func
    }
    onRecv(event) {
        if (XMessage.gi().callbackFunc != null) {
            XMessage.gi().callbackFunc(event.data.type, event.data.data)
        }
    }
    send(msgtype, msgdata) {
        if (this.messageType == XMessage.TYPE_VSCODE) {
            this.vscode.postMessage({
                type: msgtype,
                data: msgdata
            })
        }
        else if (this.messageType == XMessage.TYPE_NONE) {
            MockMessage.gi().processMessage({
                type: msgtype,
                data: msgdata
            })
        }
    }
}

XMessage.TYPE_NONE = 0
XMessage.TYPE_VSCODE = 1

XMessage.pinstance_ = null;
XMessage.gi = function () {
    if (XMessage.pinstance_ == null) {
        XMessage.pinstance_ = new XMessage();
    }
    return XMessage.pinstance_;
}

module.exports = {
    XMessage
}
