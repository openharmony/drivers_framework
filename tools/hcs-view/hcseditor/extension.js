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
// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    console.log('Congratulations, your extension "hcseditor" is now active!');
    let disposable = vscode.commands.registerCommand('hcs_editor', function (uri) {
        vscode.window.showInformationMessage('Hello World from HcsEditor!');
        if (vscode.ccPanel != undefined){
            vscode.ccPanel.dispose();
        }else {
            vscode.workspace.onDidSaveTextDocument((e)=>{
                let tt2 = fs.readFileSync(e.uri.fsPath)
                let tt = new Int8Array(tt2);
                send("freshfiledata", {
                    fn: e.uri.fsPath, data: tt
                })
                send("parse", e.uri.fsPath)})
        }
        vscode.ccPanel = vscode.window.createWebviewPanel('ccnto', "编辑"+
        path.basename(uri.fsPath), vscode.ViewColumn.Two, {
            enableScripts: true,
        });
        vscode.ccPanel.webview.html = getWebviewContent(context.extensionUri);
        vscode.ccPanel.webview.onDidReceiveMessage(msg => {
            switch (msg.type) {
                case "inited":
                    send("parse", uri.fsPath)
                    break;
                case 'getfiledata':
                    let tt2 = fs.readFileSync(msg.data);
                    let tt = new Int8Array(tt2);
                    send("filedata", {
                        fn: msg.data, data: tt
                    })
                    break;
                case 'writefile':
                    fs.writeFileSync(msg.data.fn,msg.data.data);
                    break;
                case 'selectchange':
                    vscode.workspace.openTextDocument(msg.data).then((d)=>{
                        vscode.window.showTextDocument(d,vscode.ViewColumn.One)
                    })
                    break;
                case 'open_page':
                    break;
                default:
                    vscode.window.showInformationMessage(msg.type);
                    vscode.window.showInformationMessage(msg.data);
            }
        }, undefined, context.subscriptions);
    });
    context.subscriptions.push(disposable);
}

function send(type, data) {
    vscode.ccPanel.webview.postMessage({
        type: type,
        data: data
    });
}

function getWebviewContent(extpath) {
    let uri = vscode.Uri.joinPath(extpath, "editor.html")

    let ret = fs.readFileSync(uri.fsPath, { encoding: 'utf8' });

    return ret;
}

// this method is called when your extension is deactivated
function deactivate() { }

module.exports = {
    activate,
    deactivate
}
