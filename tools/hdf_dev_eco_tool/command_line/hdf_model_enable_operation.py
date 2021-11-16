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
from string import Template
import hdf_utils

from .hdf_linux_scann import HdfLinuxScan
from .hdf_liteos_scann import HdfLiteScan


class EnableOperation(object):
    def __init__(self, root, vendor, board, model):
        self.root = root
        self.vendor = vendor
        self.board = board
        self.model = model
        self.liteos_model_name = HdfLiteScan(
            self.root, self.vendor, self.board).scan_build()
        self.linux_model_name = HdfLinuxScan(
            self.root, self.vendor, self.board).scan_makefile()

    def disable_model_liteos(self):
        dot_file_list = hdf_utils.get_dot_configs_path(
            self.root, self.vendor, self.board)
        old_template_string = \
            "LOSCFG_DRIVERS_HDF_${module_upper_case}=y"
        new_template_string = \
            "LOSCFG_DRIVERS_HDF_${module_upper_case} is not set\n"
        new_demo_config = Template(new_template_string).substitute(
            {"module_upper_case": self.model.upper()})
        old_demo_config = Template(old_template_string).substitute(
            {"module_upper_case": self.model.upper()})

        if self.model not in self.liteos_model_name:
            return False
        for dot_file in dot_file_list:
            file_lines = hdf_utils.read_file_lines(dot_file)
            for index, line in enumerate(file_lines):
                if old_demo_config == line.strip():
                    file_lines[index] = new_demo_config
            hdf_utils.write_file_lines(dot_file, file_lines)
        return True

    def enable_model_liteos(self):
        dot_file_list = hdf_utils.get_dot_configs_path(
            self.root, self.vendor, self.board)
        new_template_string = \
            "LOSCFG_DRIVERS_HDF_${module_upper_case}=y\n"
        old_template_string = \
            "LOSCFG_DRIVERS_HDF_${module_upper_case} is not set"
        new_demo_config = Template(new_template_string).substitute(
            {"module_upper_case": self.model.upper()})
        old_demo_config = Template(old_template_string).substitute(
            {"module_upper_case": self.model.upper()})

        if self.model not in self.liteos_model_name:
            return False
        for dot_file in dot_file_list:
            file_lines = hdf_utils.read_file_lines(dot_file)
            for index, line in enumerate(file_lines):
                if old_demo_config == line.strip():
                    file_lines[index] = new_demo_config
            hdf_utils.write_file_lines(dot_file, file_lines)
        return True

    def operation_enable(self):
        if self.board.split("_")[-1] != "linux":
            try:
                if self.enable_model_liteos():
                    return "success(liteos) enable %s" % self.model
                else:
                    return "%s model_name is not liteos type" % self.model
            except Exception:
                raise "failure(liteos) enable %s" % self.model
            
        else:
            try:
                if self.enable_model_linux():
                    return "success(linux) enable %s" % self.model
                else:
                    return "%s model_name is not linux type" % self.model
            except Exception:
                raise "failure(linux) enable %s" % self.model

    def operation_disable(self):
        if self.board.split("_")[-1] != "linux":
            try:
                if self.disable_model_liteos():
                    return "success(liteos) disable %s" % self.model
                else:
                    return "%s model_name is not liteos type" % self.model
            except Exception:
                raise "failure(liteos) disable %s" % self.model
        else:
            try:
                if self.disable_model_linux():
                    return "success(linux) enable %s" % self.model
                else:
                    return "%s model_name is not linux type" % self.model
            except Exception:
                raise "failure(linux) disable %s" % self.model

    def get_config_config(self, kernel):
        return os.path.join(self.root, "kernel", kernel, "config")

    def get_config_patch(self, kernel):
        return os.path.join(self.root, "kernel", kernel, "patches")

    def _get_file_patch(self, patch, endswitch, split_sign):
        file_path = []
        for roots, dirs, files in os.walk(patch):
            if endswitch == "defconfig":
                files_list = list(filter(
                    lambda x: x.split(split_sign)[-1] == endswitch, files))
            else:
                files_list = list(filter(lambda x: x == endswitch, files))
            for file in files_list:
                file_path.append(os.path.join(roots, file))
        return file_path

    def _get_config_linux(self):
        config_path = self.get_config_config(kernel="linux")
        config_path_list = self._get_file_patch(
            patch=config_path, endswitch="defconfig", split_sign="_")
        
        patch_path = self.get_config_patch(kernel="linux")
        patch_path_list = self._get_file_patch(
            patch=patch_path, endswitch="hi3516dv300.patch", split_sign=".")
        config_path_list.extend(patch_path_list)

        return config_path_list

    def _replace_operation(self, new_string, old_string, file_path):
        new_demo_config = Template(new_string).substitute(
            {"module_upper_case": self.model.upper()})
        old_demo_config = Template(old_string).substitute(
            {"module_upper_case": self.model.upper()})

        file_lines = hdf_utils.read_file_lines(file_path)
        for index, line in enumerate(file_lines):
            if old_demo_config == line.strip():
                file_lines[index] = new_demo_config
        hdf_utils.write_file_lines(file_path, file_lines)
        return True

    def disable_model_linux(self):
        if self.model not in self.linux_model_name:
            return False
        file_path_list = self._get_config_linux()
        for file_path in file_path_list:
            if file_path.split("_")[-1] == "defconfig":
                old_template_string = \
                    "CONFIG_DRIVERS_HDF_${module_upper_case}=y"
                new_template_string = \
                    "CONFIG_DRIVERS_HDF_${module_upper_case} is not set\n"
            else:
                old_template_string = \
                    "+CONFIG_DRIVERS_HDF_${module_upper_case}=y"
                new_template_string = \
                    "+CONFIG_DRIVERS_HDF_${module_upper_case} is not set\n"
            self._replace_operation(new_string=new_template_string,
                                    old_string=old_template_string,
                                    file_path=file_path)
        return True

    def enable_model_linux(self):
        if self.model not in self.linux_model_name:
            return False
        file_path_list = self._get_config_linux()
        for file_path in file_path_list:
            if file_path.split("_")[-1] == "defconfig":
                new_template_string \
                    = "CONFIG_DRIVERS_HDF_${module_upper_case}=y\n"
                old_template_string \
                    = "CONFIG_DRIVERS_HDF_${module_upper_case} is not set"
            else:
                new_template_string \
                    = "+CONFIG_DRIVERS_HDF_${module_upper_case}=y\n"
                old_template_string = \
                    "+CONFIG_DRIVERS_HDF_${module_upper_case} is not set"
            self._replace_operation(new_string=new_template_string,
                                    old_string=old_template_string,
                                    file_path=file_path)
        return True
