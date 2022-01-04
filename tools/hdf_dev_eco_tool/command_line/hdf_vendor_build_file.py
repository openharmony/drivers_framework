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

from .hdf_command_error_code import CommandErrorCode
from hdf_tool_exception import HdfToolException
import hdf_utils


class HdfVendorBuildFile(object):
    def __init__(self, root, vendor):
        self.vendor = vendor
        self.file_path = hdf_utils.get_vendor_gn_path(root)
        if not os.path.exists(self.file_path):
            raise HdfToolException('file: %s not exist' % self.file_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        self.contents = hdf_utils.read_file(self.file_path)

    def add_module(self, module):
        with open(self.file_path, 'r') as file_read:
            data = file_read.readlines()

        line_template = r'input'
        new_line = {}
        for index, line in enumerate(data):
            result = re.search(line_template, line)
            if result:
                new_line["index"] = index + 1
                new_line["value"] = line
        new_line["value"] = new_line["value"].replace("input", module)
        data.insert(new_line["index"], new_line["value"])
        with open(self.file_path, 'w') as write_file:
            for i in data:
                write_file.write(i)
        return self.file_path

    def delete_module(self, file_path, model):
        lines = hdf_utils.read_file_lines(file_path)
        for index, line in enumerate(lines):
            if line.find(model) > 0:
                del lines[index]
        hdf_utils.write_file_lines(file_path, lines)

    def rename_vendor(self):
        pattern = r'vendor/([a-zA-Z0-9_\-]+)/'
        replacement = 'vendor/%s/' % self.vendor
        new_content = re.sub(pattern, replacement, self.contents)
        hdf_utils.write_file(self.file_path, new_content)
