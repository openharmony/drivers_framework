#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import os
import sys
from string import Template

import hdf_utils
from hdf_tool_exception import HdfToolException
from .linux.kconfig_file_add_config import kconfig_file_operation
from .linux.mk_file_add_config import linux_makefile_operation

from .liteos.gn_file_add_config import build_file_operation
from .liteos.mk_file_add_config import makefile_file_operation
from ..hdf_command_error_code import CommandErrorCode
from ..hdf_defconfig_patch import HdfDefconfigAndPatch
from ..hdf_device_info_hcs import HdfDeviceInfoHcsFile



class HdfAddDriver(object):
    def __init__(self, args):
        super(HdfAddDriver, self).__init__()
        self.module = args.module_name
        self.board = args.board_name
        self.driver = args.driver_name
        self.kernel = args.kernel_name
        self.vendor = args.vendor_name
        self.root = args.root_dir
        self.template_file_path = hdf_utils.get_template_file_path(self.root)
        if not os.path.exists(self.template_file_path):
            raise HdfToolException(
                'template file: %s not exist' %
                self.template_file_path, CommandErrorCode.TARGET_NOT_EXIST)

    def add_linux(self, driver_file_path):
        adapter_hdf = hdf_utils.get_vendor_hdf_dir_adapter(
            self.root, self.kernel)
        hdf_utils.judge_file_path_exists(adapter_hdf)

        adapter_model_path = os.path.join(adapter_hdf, 'model', self.module)
        hdf_utils.judge_file_path_exists(adapter_model_path)

        liteos_file_name = ['Makefile', 'Kconfig']
        file_path = {}
        for file_name in liteos_file_name:
            if file_name == "Makefile":
                build_file_path = os.path.join(adapter_model_path, file_name)
                linux_makefile_operation(build_file_path, driver_file_path,
                                         self.module, self.driver)
                file_path['Makefile'] = build_file_path

            elif file_name == "Kconfig":
                kconfig_path = os.path.join(adapter_model_path, file_name)
                kconfig_file_operation(kconfig_path, self.module,
                                       self.driver, self.template_file_path)
                file_path['Kconfig'] = kconfig_path

        device_info = HdfDeviceInfoHcsFile(
            self.root, self.vendor, self.module, self.board, self.driver, path="")
        hcs_file_path = device_info.add_hcs_config_to_exists_model()
        file_path["devices_info.hcs"] = hcs_file_path

        template_string = "CONFIG_DRIVERS_HDF_${module_upper}_${driver_upper}=y\n"
        data_model = {
            "module_upper": self.module.upper(),
            "driver_upper": self.driver.upper()
        }

        new_demo_config = Template(template_string).substitute(data_model)
        defconfig_patch = HdfDefconfigAndPatch(
            self.root, self.vendor, self.kernel, self.board,
            data_model, new_demo_config)

        config_path = defconfig_patch.get_config_config()
        files = []
        patch_list = defconfig_patch.add_module(config_path,
                                                files=files, codetype=None)
        config_path = defconfig_patch.get_config_patch()
        files1 = []
        defconfig_list = defconfig_patch.add_module(config_path,
                                                    files=files1, codetype=None)
        file_path[self.module + "_dot_configs"] = \
            list(set(patch_list + defconfig_list))
        return file_path

    def add_liteos(self, driver_file_path):
        adapter_hdf = hdf_utils.get_vendor_hdf_dir_adapter(
            self.root, self.kernel)
        hdf_utils.judge_file_path_exists(adapter_hdf)

        adapter_model_path = os.path.join(adapter_hdf, 'model', self.module)
        hdf_utils.judge_file_path_exists(adapter_model_path)

        liteos_file_name = ['BUILD.gn', 'Makefile', 'Kconfig']
        file_path = {}
        for file_name in liteos_file_name:
            if file_name == "BUILD.gn":
                build_file_path = os.path.join(adapter_model_path, file_name)
                build_file_operation(build_file_path, driver_file_path,
                                     self.module, self.driver)
                file_path['BUILD.gn'] = build_file_path

            elif file_name == "Makefile":
                makefile_path = os.path.join(adapter_model_path, file_name)
                makefile_file_operation(makefile_path, driver_file_path,
                                        self.module, self.driver)
                file_path['Makefile'] = makefile_path

            elif file_name == "Kconfig":
                kconfig_path = os.path.join(adapter_model_path, file_name)
                kconfig_file_operation(kconfig_path, self.module,
                                       self.driver, self.template_file_path)
                file_path['Kconfig'] = kconfig_path

        # Modify hcs file
        device_info = HdfDeviceInfoHcsFile(
            self.root, self.vendor, self.module, self.board, self.driver, path="")
        hcs_file_path = device_info.add_hcs_config_to_exists_model()
        file_path["devices_info.hcs"] = hcs_file_path

        dot_file_list = hdf_utils.get_dot_configs_path(
            self.root, self.vendor, self.board)
        template_string = "LOSCFG_DRIVERS_HDF_${module_upper}_${driver_upper}=y\n"
        new_demo_config = Template(template_string).substitute(
            {"module_upper": self.module.upper(),
             "driver_upper": self.driver.upper()})

        for dot_file in dot_file_list:
            file_lines = hdf_utils.read_file_lines(dot_file)
            file_lines[-1] = file_lines[-1].strip() + "\n"
            if new_demo_config != file_lines[-1]:
                file_lines.append(new_demo_config)
                hdf_utils.write_file_lines(dot_file, file_lines)
        file_path[self.module + "_dot_configs"] = dot_file_list
        return file_path

    def driver_create_info_format(self, config_file_json, config_item, file_path):
        kernel_type = config_file_json.get(self.kernel)
        if kernel_type is None:
            config_file_json[self.kernel] = {
                config_item.get("module_name"): {
                    'module_leve_config': {},
                    "driver_file_list": {
                        config_item.get("driver_name"): config_item.get("driver_file_path")
                    }
                }
            }
            config_file_json[self.kernel][self.module]["module_leve_config"].update(file_path)
        else:
            model_type = kernel_type.get(config_item.get("module_name"))
            if model_type is None:
                temp = config_file_json.get(self.kernel)
                temp_module = config_item.get("module_name")
                temp[temp_module] = {
                    'module_leve_config': {},
                    "driver_file_list": {
                        config_item.get("driver_name"): config_item.get("driver_file_path")
                    }
                }
                config_file_json.get(self.kernel).get(self.module).get("module_leve_config").update(file_path)
            else:
                temp = config_file_json.get(self.kernel).\
                    get(config_item.get("module_name")).get("driver_file_list")
                temp[config_item.get("driver_name")] = config_item.get("driver_file_path")

        return config_file_json

    def add_driver(self, *args_tuple):
        root, vendor, module, driver, board, kernel = args_tuple
        drv_converter = hdf_utils.WordsConverter(driver)
        drv_src_dir = hdf_utils.get_drv_src_dir(root, module)
        new_mkdir_path = os.path.join(drv_src_dir, driver)
        if not os.path.exists(new_mkdir_path):
            os.mkdir(new_mkdir_path)
        data_model = {
            'driver_lower_case': drv_converter.lower_case(),
            'driver_upper_camel_case': drv_converter.upper_camel_case(),
            'driver_lower_camel_case': drv_converter.lower_camel_case(),
            'driver_upper_case': drv_converter.upper_case()
        }
        result_path = os.path.join(new_mkdir_path, '%s_driver.c' % driver)
        if os.path.exists(result_path):
            return True, result_path
        self._file_gen_lite('hdf_driver.c.template', result_path, data_model)
        result_path = os.path.join(new_mkdir_path, '%s_driver.c' % driver)
        return True, result_path

    def _file_gen_lite(self, template, source_file_path, model):
        templates_dir = hdf_utils.get_templates_lite_dir()
        template_path = os.path.join(templates_dir, template)
        self._source_template_fill(template_path, source_file_path, model)

    def _source_template_fill(self, template_path, output_path, data_model):
        if not os.path.exists(template_path):
            return
        raw_content = hdf_utils.read_file(template_path)
        contents = Template(raw_content).safe_substitute(data_model)
        hdf_utils.write_file(output_path, contents)
