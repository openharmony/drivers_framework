#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Copyright (c) 2021, Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
import sys


class IdlType:
    INTERFACE = 1
    CALLBACK = 2
    TYPES = 3


def translate_file_name(file_name):
    name = file_name[1:] if file_name.startswith("I") else file_name
    translate_name = ""
    for index,_ in enumerate(name):
        c = name[index]
        if c >= 'A' and c <= 'Z':
            if index > 1:
                translate_name += "_"
            translate_name += c.lower()
        else:
            translate_name += c
    return translate_name


def get_idl_file_type(file_path):
    idl_type = IdlType.TYPES
    file = open(file_path, "r")
    for file_line in file.readlines():
        interface_index = file_line.find("interface")
        if interface_index != -1:
            if file_line.find("[callback]", 0, interface_index) != -1:
                idl_type = IdlType.CALLBACK
            else:
                idl_type = IdlType.INTERFACE
            break
        else:
            continue
    file.close()
    return idl_type


def c_interface_file_translate(idl_file, out_dir, part, outputs):
    file = idl_file.split("/")[-1]
    file_name = translate_file_name(file.split(".")[0])

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
    file = idl_file.split("/")[-1]
    file_name = translate_file_name(file.split(".")[0])

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
    file = idl_file.split("/")[-1]
    file_name = translate_file_name(file.split(".")[0])

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
    file = idl_file.split("/")[-1]
    file_name = translate_file_name(file.split(".")[0])

    iface_header_file = os.path.join(out_dir, "i" + file_name + ".h")
    client_proxy_header_file = os.path.join(out_dir, file_name + "_proxy.h")
    client_proxy_source_file = os.path.join(out_dir, file_name + "_proxy.cpp")
    server_driver_source_file = os.path.join(out_dir,
        file_name + "_driver.cpp")
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
    file = idl_file.split("/")[-1]
    file_name = translate_file_name(file.split(".")[0])

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
    file = idl_file.split("/")[-1]
    file_name = translate_file_name(file.split(".")[0])

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


def main(argv):
    outputs = []
    if len(argv) < 4:
        return outputs

    option = argv[1]
    language = argv[2]
    out_dir = argv[3]
    files = argv[4:]

    if option == "-o":
        outputs = idl_translate(files, language, out_dir)
    elif option == "-c":
        outputs = get_compile_source_file(argv[4:],
            language, out_dir, "client_lib_source")
    elif option == "-s":
        outputs = get_compile_source_file(argv[4:],
            language, out_dir, "server_lib_source")

    sys.stdout.write('\n'.join(outputs))


if __name__ == "__main__":
    main(sys.argv)