#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
# 
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import os
import re

import hdf_utils
from hdf_tool_settings import HdfToolSettings


class HdfDefconfigAndPatch(object):
    def __init__(self, root, vendor, kernel,
                 board, data_model, new_demo_config):
        self.root = root
        self.vendor = vendor
        self.kernel = kernel
        self.board = board
        self.data_model = data_model
        self.new_demo_config = new_demo_config
        self.file_path = hdf_utils.get_template_file_path(root)
        self.drivers_path_list = HdfToolSettings().\
            get_board_parent_file(self.board)

    def get_config_config(self):
        return os.path.join(self.root, "kernel", self.kernel, "config")

    def get_config_patch(self):
        return os.path.join(self.root, "kernel", self.kernel, "patches")

    def add_module(self, path, files, codetype):
        for filename in os.listdir(path):
            new_path = os.path.join(path, filename)
            if not os.path.isdir(new_path):
                self.find_file(new_path, files, codetype)
            else:
                self.add_module(path=new_path, files=files, codetype=codetype)
        return files

    def delete_module(self, path):
        with open(path, "rb") as f_read:
            lines = f_read.readlines()
        if self.new_demo_config.encode("utf-8") in lines or \
                ("+" + self.new_demo_config).encode("utf-8") in lines:
            if path.split(".")[-1] != "patch":
                lines.remove(self.new_demo_config.encode("utf-8"))
            else:
                lines.remove(("+" + self.new_demo_config).encode("utf-8"))
        with open(path, "wb") as f_write:
            f_write.writelines(lines)

    def rename_vendor(self):
        pattern = r'vendor/([a-zA-Z0-9_\-]+)/'
        replacement = 'vendor/%s/' % self.vendor
        new_content = re.sub(pattern, replacement, self.contents)
        hdf_utils.write_file(self.file_path, new_content)

    def find_file(self, path, files, codetype):
        if path.split("\\")[-1] in self.drivers_path_list or \
                path.split("/")[-1] in self.drivers_path_list:
            files.append(path)
            if codetype is None:
                self.binary_type_write(path)
            else:
                self.utf_type_write(path, codetype)
        return files

    def binary_type_write(self, path):
        with open(path, "rb") as fread:
            data_lines_old = fread.readlines()
        data_lines = self.filter_lines(data_lines_old)
        insert_index = None
        state = False
        for index, line in enumerate(data_lines):
            if line.find("CONFIG_DRIVERS_HDF_INPUT=y".encode('utf-8')) >= 0:
                insert_index = index
            elif line.find(self.new_demo_config[-1].encode('utf-8')) >= 0:
                state = True
        if not state:
            if path.split(".")[-1] != "patch":
                for info in self.new_demo_config:
                    data_lines.insert(insert_index + 1, info.encode('utf-8'))
            else:
                for info in self.new_demo_config:
                    data_lines.insert(
                        insert_index + 1, ("+" + info).encode('utf-8'))

        with open(path, "wb") as fwrite:
            fwrite.writelines(data_lines)

    def utf_type_write(self, path, codetype):
        with open(path, "r+", encoding=codetype) as fread:
            data_lines_old = fread.readlines()
        data_lines = self.filter_lines(data_lines_old)
        insert_index = None
        state = False
        for index, line in enumerate(data_lines):
            if line.find("CONFIG_DRIVERS_HDF_INPUT=y") >= 0:
                insert_index = index
            elif line.find(self.new_demo_config[-1]) >= 0:
                state = True
        if not state:
            if path.split(".")[-1] != "patch":
                for info in self.new_demo_config:
                    data_lines.insert(insert_index + 1, info)
            else:
                for info in self.new_demo_config:
                    data_lines.insert(insert_index + 1, ("+" + info))

        with open(path, "w", encoding=codetype) as fwrite:
            fwrite.writelines(data_lines)

    def filter_lines(self, data_lines_old):
        if len(self.new_demo_config) == 2:
            temp_str = self.new_demo_config[0]
            data_lines = list(filter(lambda x: hdf_utils.judge_enable_line(
                enable_line=x, device_base=temp_str.split("=")[0]),
                                     data_lines_old))
        else:
            data_lines = data_lines_old
        return data_lines
