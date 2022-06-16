# HCS可视化工具开发说明

若当前工具功能不满足开发者需求，开发者需增强工具能力，则可基于已有源码进行工具二次开发，编译打包生成自定义的插件。

## 使用场景

开发者需增强工具能力，进行二次开发，编译打包生成自定义的插件。

## 工具开发

### 环境说明

系统：建议Ubuntu 20.04或者Windows 10

### 约束
visual studio code 版本需1.62.0及以上。

### 插件生成步骤

1. 安装依赖库

	1) 右键windows开始菜单，单击运行，输入cmd，单击确定。	
	
	
	2）在命令行中进入到hcsVSCode目录下，安装依赖库，命令如下：
	
		npm i
	
	3) 在命令行中进入到hcsWebView目录下，安装依赖库，命令如下：
	
		npm i

2. 执行脚本

	首次使用工具或本地代码有更新时，需运行hcsWebView目录下build.py文件更新本地代码，运行成功后命令行中显示“replaced”，表示代码更新成功，如下图所示：

	![](../figures/run-code-succss.png)

3. 打包

	1) 在drivers_framework/tools/hcs-view/hcsVSCode这个目录下执行命令：
	
			npm i vsce
	
	2) 在drivers_framework/tools/hcs-view/hcsVSCode这个目录下执行命令：
	
			npx vsce package
	
	3) 每个选项都选择y，然后回车，最终会在当前目录下打包生成一个插件hcseditor-0.0.1.vsix。以Ubuntu环境为例，结果如下：

			harmony@Ubuntu-64:~/hcsVSCode$ npx vsce package
			WARNING  A 'repository' field is missing from the 'package.json' manifest file.
			Do you want to continue? [y/N] y
			WARNING  Using '*' activation is usually a bad idea as it impacts performance.
			More info: https://code.visualstudio.com/api/references/activation-events#Start-up
			Do you want to continue? [y/N] y
			WARNING  LICENSE.md, LICENSE.txt or LICENSE not found
			Do you want to continue? [y/N] y
			This extension consists of 2318 files, out of which 1223 are JavaScript files. For performance reasons, you should bundle your extension: https://aka.ms/vscode-bundle-extension . You should also exclude unnecessary files by adding them to your .vscodeignore: https://aka.ms/vscode-vscodeignore
		   
## 工具测试

  [工具单元测试说明](https://gitee.com/openharmony/drivers_framework/tree/master/tools/hcs-view/hcsWebView/test/README.md)

