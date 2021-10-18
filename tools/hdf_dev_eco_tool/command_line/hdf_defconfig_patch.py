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
import re
from string import Template

from hdf_tool_settings import HdfToolSettings
from .hdf_command_error_code import CommandErrorCode
from hdf_tool_exception import HdfToolException
import hdf_utils


class HdfDefconfigAndPatch(object):
    def __init__(self, root, vendor, kernel,
                 board, data_model, new_demo_config):
        self.root = root
        self.vendor = vendor
        self.kernel = kernel
        self.board = board
        self.data_model = data_model
        self.new_demo_config = new_demo_config
        self.file_path = hdf_utils.get_template_file_path(root, vendor)
        self.drivers_path_list = HdfToolSettings().\
            get_board_parent_file(self.board)

    def get_config_config(self):
        return os.path.join(self.root, "kernel", self.kernel, "config")

    def get_config_patch(self):
        return os.path.join(self.root, "kernel", self.kernel, "patches")

        def add_module(self, path, files):
        for filename in os.listdir(path):
            new_path = os.path.join(path, filename)
            if not os.path.isdir(new_path):
                self.find_file(new_path, files)
            else:
                self.add_module(new_path, files=[])
        return files

    def delete_module(self, path):
        lines = hdf_utils.read_file_lines(path)
        if self.new_demo_config in lines or \
                ("+" + self.new_demo_config) in lines:
            if path.split(".")[-1] != "patch":
                lines.remove(self.new_demo_config)
            else:
                lines.remove("+" + self.new_demo_config)
        hdf_utils.write_file_lines(path, lines)

    def rename_vendor(self):
        pattern = r'vendor/([a-zA-Z0-9_\-]+)/'
        replacement = 'vendor/%s/' % self.vendor
        new_content = re.sub(pattern, replacement, self.contents)
        hdf_utils.write_file(self.file_path, new_content)

    def find_file(self, path, files):
        if path.split("\\")[-1] in self.drivers_path_list or \
                path.split("/")[-1] in self.drivers_path_list:
            files.append(path)
            codetype = "utf-8"
            with open(path, "r+", encoding=codetype) as fread:
                data = fread.readlines()
            insert_index = None
            state = False
            for index, line in enumerate(data):
                if line.find("CONFIG_DRIVERS_HDF_INPUT=y") >= 0:
                    insert_index = index
                elif line.find(self.new_demo_config) >= 0:
                    state = True
            if not state:
                if path.split(".")[-1] != "patch":
                    data.insert(insert_index + 1,
                                self.new_demo_config)
                else:
                    data.insert(insert_index + 1,
                                "+" + self.new_demo_config)
            with open(path, "w", encoding=codetype) as fwrite:
                fwrite.writelines(data)
