#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import os
import re
import sys


class IdlType(object):
    INTERFACE = 1
    CALLBACK = 2
    TYPES = 3


def translate_file_name(file_name):
    name = file_name[1:] if file_name.startswith("I") else file_name
    translate_name = ""
    for i in range(len(name)):
        c = name[i]
        if c >= 'A' and c <= 'Z':
            if i > 1:
                translate_name += "_"
            translate_name += c.lower()
        else:
            translate_name += c
    return translate_name


def get_idl_file_type(file_path):
    idl_type = IdlType.TYPES
    file_option = open(file_path, "r")
    for file_line in file_option.readlines():
        interface_index = file_line.find("interface")
        if interface_index != -1:
            if file_line.find("[callback]", 0, interface_index) != -1:
                idl_type = IdlType.CALLBACK
            else:
                idl_type = IdlType.INTERFACE
            break
        else:
            continue
    file_option.close()
    return idl_type


def c_interface_file_translate(idl_file, out_dir, part, outputs):
    get_file_name = idl_file.split("/")[-1]
    file_name = translate_file_name(get_file_name.split(".")[0])

    iface_header_file = os.path.join(out_dir, "i" + file_name + ".h")
    client_proxy_source_file = os.path.join(out_dir, file_name + "_proxy.c")
    server_driver_source_file = os.path.join(out_dir, file_name + "_driver.c")
    server_stub_header_file = os.path.join(out_dir, file_name + "_stub.h")
    server_stub_source_file = os.path.join(out_dir, file_name + "_stub.c")
    server_impl_header_file = os.path.join(out_dir, file_name + "_service.h")
    server_impl_source_file = os.path.join(out_dir, file_name + "_service.c")

    if part == "client_lib_source":
        outputs.append(iface_header_file)
        outputs.append(client_proxy_source_file)
    elif part == "server_lib_source":
        outputs.append(iface_header_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
    else:
        outputs.append(iface_header_file)
        outputs.append(client_proxy_source_file)
        outputs.append(server_driver_source_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
        outputs.append(server_impl_header_file)
        outputs.append(server_impl_source_file)


def c_callback_file_translate(idl_file, out_dir, part, outputs):
    get_file_name = idl_file.split("/")[-1]
    file_name = translate_file_name(get_file_name.split(".")[0])

    iface_header_file = os.path.join(out_dir, "i" + file_name + ".h")
    client_proxy_source_file = os.path.join(out_dir, file_name + "_proxy.c")
    server_stub_header_file = os.path.join(out_dir, file_name + "_stub.h")
    server_stub_source_file = os.path.join(out_dir, file_name + "_stub.c")
    server_impl_header_file = os.path.join(out_dir, file_name + "_service.h")
    server_impl_source_file = os.path.join(out_dir, file_name + "_service.c")

    if part == "client_lib_source":
        outputs.append(iface_header_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
    elif part == "server_lib_source":
        outputs.append(iface_header_file)
        outputs.append(client_proxy_source_file)
    else:
        outputs.append(iface_header_file)
        outputs.append(client_proxy_source_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
        outputs.append(server_impl_header_file)
        outputs.append(server_impl_source_file)


def c_types_file_translate(idl_file, out_dir, outputs):
    get_file_name = idl_file.split("/")[-1]
    file_name = translate_file_name(get_file_name.split(".")[0])

    types_header_file = os.path.join(out_dir, file_name + ".h")
    types_source_file = os.path.join(out_dir, file_name + ".c")

    outputs.append(types_header_file)
    outputs.append(types_source_file)


def c_idl_translate(idl_files, out_dir):
    outputs = []
    for idl_file in idl_files:
        idl_file_type = get_idl_file_type(idl_file)
        if idl_file_type == IdlType.INTERFACE:
            c_interface_file_translate(idl_file, out_dir, "all", outputs)
        elif idl_file_type == IdlType.CALLBACK:
            c_callback_file_translate(idl_file, out_dir, "all", outputs)
        elif idl_file_type == IdlType.TYPES:
            c_types_file_translate(idl_file, out_dir, outputs)
    return outputs


def cpp_interface_file_translate(idl_file, out_dir, part, outputs):
    get_file_name = idl_file.split("/")[-1]
    file_name = translate_file_name(get_file_name.split(".")[0])

    iface_header_file = os.path.join(out_dir, "i" + file_name + ".h")
    client_proxy_header_file = os.path.join(out_dir, file_name + "_proxy.h")
    client_proxy_source_file = os.path.join(out_dir, file_name + "_proxy.cpp")
    server_driver_source_file = os.path.join(out_dir, file_name + "_driver.cpp")
    server_stub_header_file = os.path.join(out_dir, file_name + "_stub.h")
    server_stub_source_file = os.path.join(out_dir, file_name + "_stub.cpp")
    server_impl_header_file = os.path.join(out_dir, file_name + "_service.h")
    server_impl_source_file = os.path.join(out_dir, file_name + "_service.cpp")

    if part == "client_lib_source":
        outputs.append(iface_header_file)
        outputs.append(client_proxy_header_file)
        outputs.append(client_proxy_source_file)
    elif part == "server_lib_source":
        outputs.append(iface_header_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
    else:
        outputs.append(iface_header_file)
        outputs.append(client_proxy_header_file)
        outputs.append(client_proxy_source_file)
        outputs.append(server_driver_source_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
        outputs.append(server_impl_header_file)
        outputs.append(server_impl_source_file)


def cpp_callback_file_translate(idl_file, out_dir, part, outputs):
    get_file_name = idl_file.split("/")[-1]
    file_name = translate_file_name(get_file_name.split(".")[0])

    iface_header_file = os.path.join(out_dir, "i" + file_name + ".h")
    client_proxy_header_file = os.path.join(out_dir, file_name + "_proxy.h")
    client_proxy_source_file = os.path.join(out_dir, file_name + "_proxy.cpp")
    server_stub_header_file = os.path.join(out_dir, file_name + "_stub.h")
    server_stub_source_file = os.path.join(out_dir, file_name + "_stub.cpp")
    server_impl_header_file = os.path.join(out_dir, file_name + "_service.h")
    server_impl_source_file = os.path.join(out_dir, file_name + "_service.cpp")

    if part == "client_lib_source":
        outputs.append(iface_header_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
    elif part == "server_lib_source":
        outputs.append(iface_header_file)
        outputs.append(client_proxy_header_file)
        outputs.append(client_proxy_source_file)
    else:
        outputs.append(iface_header_file)
        outputs.append(client_proxy_header_file)
        outputs.append(client_proxy_source_file)
        outputs.append(server_stub_header_file)
        outputs.append(server_stub_source_file)
        outputs.append(server_impl_header_file)
        outputs.append(server_impl_source_file)


def cpp_types_file_translate(idl_file, out_dir, outputs):
    get_file_name = idl_file.split("/")[-1]
    file_name = translate_file_name(get_file_name.split(".")[0])

    types_header_file = os.path.join(out_dir, file_name + ".h")
    types_source_file = os.path.join(out_dir, file_name + ".cpp")

    outputs.append(types_header_file)
    outputs.append(types_source_file)


def cpp_idl_translate(idl_files, out_dir):
    outputs = []
    for idl_file in idl_files:
        idl_file_type = get_idl_file_type(idl_file)
        if idl_file_type == IdlType.INTERFACE:
            cpp_interface_file_translate(idl_file, out_dir, "all", outputs)
        elif idl_file_type == IdlType.CALLBACK:
            cpp_callback_file_translate(idl_file, out_dir, "all", outputs)
        elif idl_file_type == IdlType.TYPES:
            cpp_types_file_translate(idl_file, out_dir, outputs)   
    return outputs


def idl_translate(idl_files, language, out_dir):
    outputs = []
    if language == "c":
        outputs = c_idl_translate(idl_files, out_dir)
    elif language == "cpp":
        outputs = cpp_idl_translate(idl_files, out_dir)
    return outputs


def c_get_compile_source_file(idl_files, out_dir, part):
    outputs = []
    for idl_file in idl_files:
        idl_file_type = get_idl_file_type(idl_file)
        if idl_file_type == IdlType.INTERFACE:
            c_interface_file_translate(idl_file, out_dir, part, outputs)
        elif idl_file_type == IdlType.CALLBACK:
            c_callback_file_translate(idl_file, out_dir, part, outputs)
        elif idl_file_type == IdlType.TYPES:
            c_types_file_translate(idl_file, out_dir, outputs)
    return outputs


def cpp_get_compile_source_file(idl_files, out_dir, part):
    outputs = []
    for idl_file in idl_files:
        idl_file_type = get_idl_file_type(idl_file)
        if idl_file_type == IdlType.INTERFACE:
            cpp_interface_file_translate(idl_file, out_dir, part, outputs)
        elif idl_file_type == IdlType.CALLBACK:
            cpp_callback_file_translate(idl_file, out_dir, part, outputs)
        elif idl_file_type == IdlType.TYPES:
            cpp_types_file_translate(idl_file, out_dir, outputs)
    return outputs


def get_compile_source_file(idl_files, language, out_dir, part):
    outputs = []
    if language == "c":
        outputs = c_get_compile_source_file(idl_files, out_dir, part)
    elif language == "cpp":
        outputs = cpp_get_compile_source_file(idl_files, out_dir, part)
    return outputs


def get_files(argv):
    outputs = []
    if len(argv) < 4:
        return outputs

    option_mode = argv[1]
    language = argv[2]
    out_dir = argv[3]
    files = argv[4:]

    if option_mode == "-o":
        outputs = idl_translate(files, language, out_dir)
    elif option_mode == "-c":
        outputs = get_compile_source_file(argv[4:], language, out_dir, "client_lib_source")
    elif option_mode == "-s":
        outputs = get_compile_source_file(argv[4:], language, out_dir, "server_lib_source")

    sys.stdout.write('\n'.join(outputs))


def get_file_version(file_path):
    major_version = 0
    minor_version = 0
    file_option = open(file_path, "r")
    file_str = file_option.read()
    result = re.findall(r'package\s\w+(?:\.\w+)*\.[V|v](\d+)_(\d+);', file_str)

    if len(result) > 0:
        major_version = result[0][0]
        minor_version = result[0][1]
    file_option.close()
    version = str(major_version) + "." + str(minor_version)
    return version


def get_version(argv):
    version = "0.0"
    idl_files = argv[2:]
    for idl_file in idl_files:
        idl_file_type = get_idl_file_type(idl_file)
        if idl_file_type == IdlType.INTERFACE:
            version = get_file_version(idl_file)
            break
    sys.stdout.write(version)


if __name__ == "__main__":
    if len(sys.argv) < 1:
        sys.stdout.write('\n')
    option = sys.argv[1]
    if option == "-v":
        get_version(sys.argv)
    else:
        get_files(sys.argv)