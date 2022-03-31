#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
# 
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import os
import re
from string import Template

import hdf_utils
from hdf_tool_exception import HdfToolException
from hdf_tool_settings import HdfToolSettings
from .hdf_command_error_code import CommandErrorCode


class HdfDeviceInfoHcsFile(object):
    def __init__(self, root, vendor, module, board, driver, path):
        if not path:
            self.module = module
            self.vendor = vendor
            self.board = board
            self.root = root
            self.driver = driver
            self.lines = None
            board_parent_path = HdfToolSettings().get_board_parent_path(board)
            self.hcspath = os.path.join(self.root, board_parent_path, "device_info.hcs")
        else:
            self.hcspath = path
            self.root = root
        self.file_path = hdf_utils.get_template_file_path(root)
        if not os.path.exists(self.file_path):
            raise HdfToolException(
                'template file: %s not exist' %
                self.file_path, CommandErrorCode.TARGET_NOT_EXIST)
        if not os.path.exists(self.hcspath):
            raise HdfToolException(
                'hcs file: %s not exist' %
                self.hcspath, CommandErrorCode.TARGET_NOT_EXIST)
        self.data = {
            "driver_name": self.driver,
            "model_name": self.module,
        }

    def _save(self):
        if self.lines:
            codetype = "utf-8"
            with open(self.hcspath, "w+", encoding=codetype) as lwrite:
                for line in self.lines:
                    lwrite.write(line)

    def _find_line(self, pattern):
        for index, line in enumerate(self.lines):
            if re.search(pattern, line):
                return index, line
        return 0, ''

    def _find_last_include(self):
        if not self.lines:
            return 0
        i = len(self.lines) - 1
        while i >= 0:
            line = self.lines[i]
            if re.search(self.include_pattern, line):
                return i + 1
            i -= 1
        return 0

    def _create_makefile(self):
        mk_path = os.path.join(self.file_dir, 'Makefile')
        template_str = hdf_utils.get_template('hdf_hcs_makefile.template')
        hdf_utils.write_file(mk_path, template_str)

    def check_and_create(self):
        if self.lines:
            return
        if not os.path.exists(self.file_dir):
            os.makedirs(self.file_dir)
        self._create_makefile()
        self.lines.append('#include "hdf_manager/manager_config.hcs"\n')
        self._save()

    def add_driver(self, module, driver):
        target_line = self.line_template % (module, driver)
        target_pattern = self.line_pattern % (module, driver)
        idx, line = self._find_line(target_pattern)
        if line:
            self.lines[idx] = target_line
        else:
            pos = self._find_last_include()
            self.lines.insert(pos, target_line)
        self._save()

    def delete_driver(self, module):
        hcs_config = hdf_utils.read_file_lines(self.hcspath)
        index_info = {}
        count = 0
        for index, line in enumerate(hcs_config):
            if line.find("%s :: host" % module) > 0:
                index_info["start_index"] = index
                for child_index in range(
                        index_info.get("start_index"), len(hcs_config)):
                    if hcs_config[child_index].strip().find("{") != -1:
                        count += 1
                    elif hcs_config[child_index].strip() == "}":
                        count -= 1
                    if count == 0:
                        index_info["end_index"] = child_index
                        break
                break
        if index_info:
            self.lines = hcs_config[0:index_info.get("start_index")] \
                         + hcs_config[index_info.get("end_index") + 1:]
            self._save()
            return True

    def add_model_hcs_file_config(self):
        template_path = os.path.join(self.file_path,
                                     'device_info_hcs.template')
        lines = list(map(lambda x: "\t\t" + x,
                         hdf_utils.read_file_lines(template_path)))
        old_lines = list(filter(lambda x: x != "\n",
                                hdf_utils.read_file_lines(self.hcspath)))

        new_data = old_lines[:-2] + lines + old_lines[-2:]
        for index, _ in enumerate(new_data):
            new_data[index] = Template(new_data[index]).substitute(self.data)

        self.lines = new_data
        self._save()
        return self.hcspath

    def add_model_hcs_file_config_user(self):
        template_path = os.path.join(self.file_path,
                                     'User_device_info_hcs.template')
        lines = list(map(lambda x: "\t\t" + x,
                         hdf_utils.read_file_lines(template_path)))
        lines[-1] = "\t\t"+lines[-1].strip()+"\n"
        old_lines = list(filter(lambda x: x != "\n",
                                hdf_utils.read_file_lines(self.hcspath)))

        new_data = old_lines[:-2] + lines + old_lines[-2:]
        for index, _ in enumerate(new_data):
            new_data[index] = Template(new_data[index]).substitute(self.data)

        self.lines = new_data
        self._save()
        return self.hcspath

    def add_hcs_config_to_exists_model(self):
        template_path = os.path.join(self.file_path,
                                     'exists_model_hcs_info.template')
        lines = list(map(lambda x: "\t\t\t" + x,
                         hdf_utils.read_file_lines(template_path)))
        old_lines = list(filter(lambda x: x != "\n",
                                hdf_utils.read_file_lines(self.hcspath)))

        end_index, start_index = self._get_model_index(old_lines)
        model_hcs_lines = old_lines[start_index:end_index]
        hcs_judge = self.judge_driver_hcs_exists(date_lines=model_hcs_lines)
        if hcs_judge:
            return self.hcspath
        for index, _ in enumerate(lines):
            lines[index] = Template(lines[index]).substitute(self.data)

        self.lines = old_lines[:end_index] + lines + old_lines[end_index:]
        self._save()
        return self.hcspath

    def _get_model_index(self, old_lines):
        model_start_index = 0
        model_end_index = 0
        start_state = False
        count = 0
        for index, old_line in enumerate(old_lines):
            if old_line.strip().startswith(self.module):
                model_start_index = index
                count += 1
                start_state = True
            else:
                if start_state and old_line.find("{") != -1:
                    count += 1
                elif start_state and old_line.find("}") != -1:
                    count -= 1
                    if count == 0:
                        start_state = False
                        model_end_index = index
        return model_end_index, model_start_index

    def judge_driver_hcs_exists(self, date_lines):
        for _, line in enumerate(date_lines):
            if line.startswith("#"):
                continue
            elif line.find(self.driver) != -1:
                return True
        return False
