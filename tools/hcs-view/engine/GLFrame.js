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

import { X2DFast } from "./graphics/X2DFast.js";
import { Scr } from "./XDefine.js";

export var gl;

function touchStart(e) { 
    e.preventDefault(); 
    GLFrame.pinstance_.callbackProctouch(1, e.touches[0].clientX, e.touches[0].clientY);
}
function touchMove(e) { 
    e.preventDefault(); 
    GLFrame.pinstance_.callbackProctouch(2, e.touches[0].clientX, e.touches[0].clientY);
}
function touchEnd(e) { 
    e.preventDefault(); 
    GLFrame.pinstance_.callbackProctouch(3, e.changedTouches[0].clientX, e.changedTouches[0].clientY);
}

function mouseDown(e) { 
    e.preventDefault(); 
    GLFrame.pinstance_.callbackProctouch(1, e.offsetX, e.offsetY) 
}
function mouseMove(e) { 
    e.preventDefault(); 
    GLFrame.pinstance_.callbackProctouch(2, e.offsetX, e.offsetY) 
}
function mouseUp(e) { 
    e.preventDefault(); 
    GLFrame.pinstance_.callbackProctouch(3, e.offsetX, e.offsetY) 
}

function keyDown(e) {
    let ret = ""
    if (e.ctrlKey) {
        if (ret.length > 0) ret += "+"
        ret += "ctrl"
    }
    if (e.shiftKey) {
        if (ret.length > 0) ret += "+"
        ret += "shift"
    }
    if (e.altKey) {
        if (ret.length > 0) ret += "+"
        ret += "alt"
    }
    if (ret.length > 0) ret += "+"
    ret += e.key
    GLFrame.pinstance_.callbackKey(1, ret)
}

function mainLoop() {
    GLFrame.pinstance_.callbackDraw();
    window.requestAnimationFrame(mainLoop)
}

export class GLFrame {
    static gi() {
        return GLFrame.pinstance_;
    }
    constructor() {
    }

    go(cvs, _draw = null, _touch = null, _key = null, _logic = null) {
        gl = cvs.getContext('webgl', { premultipliedAlpha: false });

        this.pCallbackDraw = _draw
        this.pCallbackTouch = _touch
        this.pCallbackKey = _key
        this.pCallbackLogic = _logic

        cvs.addEventListener("touchstart", touchStart);
        cvs.addEventListener("touchmove", touchMove);
        cvs.addEventListener("touchend", touchEnd);

        cvs.addEventListener("mousedown", mouseDown);
        cvs.addEventListener("mousemove", mouseMove);
        cvs.addEventListener("mouseup", mouseUp);

        window.addEventListener('keydown', keyDown);
        window.requestAnimationFrame(mainLoop)
    }
    callbackKey(type, code) {
        if (this.pCallbackKey != null) {
            this.pCallbackKey(type, code)
        }
    }
    callbackDraw() {
        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        if (this.pCallbackDraw != null) {
            this.pCallbackDraw();
        }
    }
    callbackProctouch(msg, x, y) {
        if (this.pCallbackTouch != null) {
            x = x * Scr.logicw / Scr.width
            y = y * Scr.logich / Scr.height
            this.pCallbackTouch(msg, x, y);
        }
    }
    resize() {
        gl.viewport(0, 0, Scr.logicw, Scr.logich);
        X2DFast.gi().resetMat();
    }
}

GLFrame.pinstance_ = new GLFrame()