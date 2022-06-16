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
const { NapiLog } = require("./../hcs/NapiLog");
let mockTest = [{
    fn: "D:\\ceshi\\d.hcs",
    data: `
	#include "D:\\ceshi\\b.hcs"
root {
    nodeaa {
		childnode {
		}
		childatt = "";
    }
	nodebb {
	}
	extint = 0;	
	extstring = "";
 }`
},
{
    fn: "D:\\ceshi\\b.hcs",
    data: `
root {
	 nodeaa {
    }
	nodebb {
	}
 }`
}
]

function getArray(fn) {
    for (let i in mockTest) {
        if (mockTest[i].fn == fn) {
            let ret = []
            for (let j in mockTest[i].data) {
                ret.push(mockTest[i].data.charCodeAt(j))
            }
            return ret
        }
    }
    return null;
}

class MockMessage {
    constructor() {
    }
    send(type, data) {
        setTimeout(() => {
            const { XMessage } = require("./XMessage");
            XMessage.gi().onRecv({
                data: {
                    type: type,
                    data: data
                }
            })
        }, 100);
    }
    processMessage(msg) {
        NapiLog.logInfo("---MockMessage start---")
        NapiLog.logInfo(msg.type)
        NapiLog.logInfo(msg.data)
        if (msg.type == "inited") {//After initialization, send the file name for resolution
            this.send("parse", mockTest[0].fn);
        }
        else if (msg.type == "getfiledata") {//Get file data
            this.send("filedata", {
                fn: msg.data,
                data: getArray(msg.data)
            })
        }
        NapiLog.logInfo("---MockMessage end---")
    }
}

MockMessage.pInstance_ = null;
MockMessage.gi = function () {
    if (MockMessage.pInstance_ == null) MockMessage.pInstance_ = new MockMessage();
    return MockMessage.pInstance_;
}

module.exports = {
    MockMessage
}