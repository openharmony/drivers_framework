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

function search(ss, data) {
    ss = replaceAll(ss, "\\.", "\\.")
    let reg = new RegExp(ss);
    let tt = reg.exec(data);
    if (tt == null) return null;
    let ret = { "regs": [] }
    for (let i = 0; i < tt.length; i++) {
        let p = data.indexOf(tt[i]);
        if (tt[i] == null) {
            ret["regs"].push([-1, -1])
        }
        else {
            ret["regs"].push([p, p + tt[i].length])
        }
    }

    return ret;
}

function match(ss, data) {
    let tt = search(ss, data)
    if (tt != null && tt.regs[0][0] == 0) return tt;
    return null;
}

function removeReg(data, reg) {
    return data.substring(0, reg[0]) + data.substring(reg[1], data.length)
}

function getReg(data, reg) {
    return data.substring(reg[0], reg[1])
}

function all(sfrom) {
    return new RegExp(sfrom, "g");
}

function replaceAll(ss, sfrom, sto) {
    return ss.replace(all(sfrom), sto)
}

module.exports = {
    search,
    match,
    removeReg,
    getReg,
    replaceAll,
    all
}