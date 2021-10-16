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

from .hdf_command_error_code import CommandErrorCode
from hdf_tool_exception import HdfToolException
import hdf_utils


class HdfVendorMakeFile(object):
    def __init__(self, root, vendor, kernel, path):
        if path:
            self.file_path = path
        else:
            self.vendor = vendor
            self.kernel = kernel
            self.file_path = hdf_utils.\
                get_vendor_makefile_path(root, self.kernel)
        if not os.path.exists(self.file_path):
            raise HdfToolException('file: %s not exist' % self.file_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        self.contents = hdf_utils.read_file_lines(self.file_path)
        self.template_str = \
            "obj-$(CONFIG_DRIVERS_HDF_${module_upper_case}) " \
            "+= model/${module_lower_case}/\n"

    def _begin_end(self, module):
        module_id = hdf_utils.get_id(self.vendor, module)
        begin = '\n# <begin %s\n' % module_id
        end = '\n# %s end>\n' % module_id
        return begin, end

    def add_module(self, data_model):
        self.contents.append(Template(self.template_str)
                             .safe_substitute(data_model))
        hdf_utils.write_file_lines(self.file_path, self.contents)
        return self.file_path

    def delete_module(self, module):
        module_converter = hdf_utils.WordsConverter(module)
        data_model = {
            'module_upper_case': module_converter.upper_case(),
            'module_lower_case': module_converter.lower_case(),
        }
        delete_template = Template(
            self.template_str).safe_substitute(data_model)
        if delete_template in self.contents:
            self.contents.remove(delete_template)
            hdf_utils.write_file_lines(
                content=self.contents, file_path=self.file_path)

    def rename_vendor(self):
        pattern = r'vendor/([a-zA-Z0-9_\-]+)/'
        replacement = 'vendor/%s/' % self.vendor
        new_content = re.sub(pattern, replacement, self.contents)
        hdf_utils.write_file(self.file_path, new_content)
