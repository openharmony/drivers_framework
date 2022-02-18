# h2idl工具使用手册

- [工具简介](#工具简介)
  - [用途](#用途)
  - [需求背景](#需求背景)
  - [IDL简介](#IDL简介)
- [工具使用](#工具使用)
  - [安装python](#安装python)
  - [使用方法](#使用方法)
  - [参数说明](#参数说明)
  - [命令举例](#命令举例)

  
## 工具简介
### 用途

本工具用于将C/C++头文件(.h)转换为接口描述文件(.idl)

### 需求背景

对于存量接口或者已有C/C++接口，开发者期望能提供工具可以自动将C/C++头文件转换为idl文件，以提高HDI开发效率。

### IDL简介

​		当客户端与服务器通信时，需要定义双方都认可的接口，以保障双方可以成功通信。HarmonyOs IDL(Interface Definition Language) 则是一种定义此类接口的语言。

​		HarmonyOS IDL先把需要传递的对象分解成操作系统能够理解的基本类型，并根据开发者的需要封装跨边界的对象。在HarmonyOS中，HarmonyOS IDL接口包含面向应用程序的北向接口和面向硬件设备的南向接口。

HarmonyOs IDL接口描述语言主要用于：

- 声明系统服务对外提供的服务接口，根据接口声明在编译时生成跨进程调用（IPC）或跨设备调用（RPC）的代理（Proxy）和桩（Stub）的C/C++代码。
- 声明Ability对外提供的服务接口，根据接口声明在编译时生成跨进程调用（IPC）或跨设备调用（RPC）的代理（Proxy）和桩（Stub）的C/C++代码。

使用HarmonyOS IDL接口描述语言声明接口具有以下优点：

- HarmonyOS IDL中是以接口的形式定义服务，可以专注于定义而隐藏实现细节。
- HarmonyOS IDL中定义的接口可以支持跨进程调用或跨设备调用。根据HarmonyOS IDL中的定义生成的信息或代码可以简化跨进程或跨设备调用接口的实现。

## 工具使用
### 安装python
本工具需要安装python 3.8及以上版本，请根据环境选择下载对应的安装包。Python下载地址：https://www.python.org/downloads/

### 使用方法

```bash
python idl_generator.py -f <*.h> -o <directory>
```

### 参数说明
```
  -h, --help            show this help message and exit
  -v, --version         Display version information
  -f <*.h>, --file <*.h>
                        Compile the C/C++ header file
  -o <directory>, --out <directory>
                        Place generated .idl files into the <directory>
```

### 命令举例
```bash
python idl_generator.py -f ./h/audio/audio_adapter.h -o out
```