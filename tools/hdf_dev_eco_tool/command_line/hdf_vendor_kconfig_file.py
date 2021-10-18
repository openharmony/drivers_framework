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

from hdf_tool_settings import HdfToolSettings
import hdf_utils


class HdfVendorKconfigFile(object):
    def __init__(self, root, vendor, kernel, path):
        if not path:
            self.kernel = kernel
            self.root = root
            self.vendor = vendor
            self.kconfig_path = hdf_utils.\
                get_vendor_kconfig_path(self.root, self.kernel)
        else:
            self.kconfig_path = path
        if os.path.exists(self.kconfig_path):
            self.lines = hdf_utils.read_file_lines(self.kconfig_path)
        else:
            self.lines = []
        drivers_path = HdfToolSettings().get_drivers_path_adapter()
        if kernel.capitalize() == "Liteos" or re.search(
                r'liteos/Kconfig', self.kconfig_path):
            line_pattern = \
                r'^\s*source\s+"../../(%s/([a-zA-Z0-9_\-]+)/.*)"'\
                % (drivers_path)
            self.line_re = re.compile(line_pattern)
            self.line_prefix = 'source "../../%s/%s/%s"'\
                               % (drivers_path, 'liteos', 'model')
        elif kernel.capitalize() == "Linux" or re.search(
                r'linux/Kconfig', self.kconfig_path):
            line_pattern = \
                r'^\s*source\s+"../../(%s/([a-zA-Z0-9_\-]+)/.*)"'\
                % (drivers_path)
            self.line_re = re.compile(line_pattern)
            self.line_prefix = 'source "drivers/hdf/khdf/model/%s/Kconfig"\n'

    def _save(self):
        if self.lines:
            hdf_utils.write_file(self.kconfig_path, ''.join(self.lines))

    def get_module_and_config_path(self):
        tuples = []
        for line in self.lines:
            match_obj = self.line_re.search(line)
            if match_obj:
                k_path_raw = match_obj.group(1).replace('/', os.sep)
                k_path = os.path.join(self.root, k_path_raw)
                tuples.append((match_obj.group(2), k_path))
        return tuples

    def _find_line(self, line_part):
        for index, line in enumerate(self.lines):
            if line_part in line:
                return index, line
        return 0, ''

    def add_module(self, module_to_k_path_parts):
        module_k_part = '/'.join(module_to_k_path_parts)
        index, line = self._find_line(module_k_part)
        if line:
            return
        line = '\n%s/%s"\n' % (self.line_prefix, module_k_part)
        if self.lines:
            if self.lines[-1].endswith('\n'):
                if self.kernel.capitalize() == "Liteos":
                    line = '%s/%s"\n' % (self.line_prefix[:-1], module_k_part)
                elif self.kernel.capitalize() == "Linux":
                    line = self.line_prefix % (module_to_k_path_parts[0])
        for file_index, file_line in enumerate(self.lines):
            if file_line.strip() == "endif":
                self.lines.insert(file_index - 1, line)
                break
        self._save()
        return self.kconfig_path

    def delete_module(self, module):
        if self.line_prefix.find("Kconfig") > 0:
            line_part = self.line_prefix % module
        else:
            line_part = '%s/%s/Kconfig"\n' % (self.line_prefix[:-1], module)
        index, line = self._find_line(line_part)
        if line:
            self.lines[index] = ''
        self._save()

    def rename_vendor(self, ):
        for i, line in enumerate(self.lines):
            pattern = r'vendor/([a-zA-Z0-9_\-]+)/'
            replacement = 'vendor/%s/' % self.vendor
            self.lines[i] = re.sub(pattern, replacement, line)
        self._save()
