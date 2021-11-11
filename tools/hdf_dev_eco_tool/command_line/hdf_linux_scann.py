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
import json
import os
import re

from .hdf_command_error_code import CommandErrorCode
from hdf_tool_exception import HdfToolException
import hdf_utils


class HdfLinuxScan(object):
    def __init__(self, root, vendor, board):
        self.root = root
        self.vendor = vendor
        self.board = board
        self.kernel = "linux"

        self.Makefile_path = hdf_utils.get_vendor_makefile_path(
            root, kernel="linux")
        if not os.path.exists(self.Makefile_path):
            raise HdfToolException('Makefile: %s not exist' % self.Makefile_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)

        self.framework_dir = hdf_utils.get_module_dir(self.root, vendor)
        if not os.path.exists(self.framework_dir):
            raise HdfToolException('file: %s not exist' % self.framework_dir,
                                   CommandErrorCode.TARGET_NOT_EXIST)

        self.hcs_path = hdf_utils.get_hcs_file_path(
            self.root, self.vendor, self.board)
        if not os.path.exists(self.hcs_path):
            raise HdfToolException('file: %s not exist' % self.hcs_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        self.contents = hdf_utils.read_file_lines(self.Makefile_path)
        self.re_temp2 = r'model/[a-z 0-9]+'

    def scan_makefile(self):
        model_list = []
        for i in self.contents:
            result = re.search(self.re_temp2, i)
            if result:
                model_name = result.group().split('/')[-1]
                if model_name not in model_list:
                    model_list.append(model_name)
        return list(set(model_list))

    def _get_model_file_dict(self):
        model_file_dict = {}
        for model_name in self.scan_makefile():
            model_file_dict[model_name] = []
            path = os.path.join(self.framework_dir, model_name)
            for root_path, dirs, files in os.walk(path):
                for file in files:
                    model_file_dict[model_name].append(
                        os.path.join(root_path, file))
        return model_file_dict

    def get_model_scan(self):
        model_dict = {}
        linux_model_info = {}
        model_list = self.scan_makefile()
        for model_name in model_list:
            adapter_model_path = os.path.join(
                hdf_utils.get_vendor_hdf_dir_adapter(
                    self.root, self.kernel), 'model', model_name)
            if os.path.exists(adapter_model_path):
                model_dict[model_name] = {}
                model_config_list = []
                for roots, dirs, files in os.walk(adapter_model_path):
                    for file in files:
                        model_config_list.append(os.path.join(roots, file))
                    model_dict[model_name]["model_configs"] = model_config_list
                    model_dict[model_name]['model_drivers'] = \
                        self._get_model_file_dict()[model_name]

        linux_model_info["model_dict"] = model_dict
        linux_model_info["linux_hcs"] = self.hcs_path

        return json.dumps(linux_model_info, indent=4)