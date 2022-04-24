#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import configparser
import os
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
        self.device = args.device_name
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

        device_info = HdfDeviceInfoHcsFile(self.root, self.vendor,
                                           self.module, self.board,
                                           self.driver, path="")
        hcs_file_path = device_info.add_hcs_config_to_exists_model()
        file_path["devices_info.hcs"] = hcs_file_path
        device_enable_config_line = self.__get_enable_config()
        template_string = "CONFIG_DRIVERS_HDF_${module_upper}_${driver_upper}=y\n"
        data_model = {
            "module_upper": self.module.upper(),
            "driver_upper": self.driver.upper()
        }

        new_demo_config = Template(template_string).substitute(data_model)
        if device_enable_config_line:
            new_demo_config_list = [device_enable_config_line, new_demo_config]
        else:
            new_demo_config_list = [new_demo_config]
        defconfig_patch = HdfDefconfigAndPatch(
            self.root, self.vendor, self.kernel, self.board,
            data_model, new_demo_config_list)

        config_path = defconfig_patch.get_config_config()
        files = []
        patch_list = defconfig_patch.add_module(
            config_path, files=files, codetype=None)
        config_path = defconfig_patch.get_config_patch()
        files1 = []
        defconfig_list = defconfig_patch.add_module(
            config_path, files=files1, codetype=None)
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
            self.root, self.vendor, self.module,
            self.board, self.driver, path="")
        hcs_file_path = device_info.add_hcs_config_to_exists_model()
        file_path["devices_info.hcs"] = hcs_file_path

        dot_file_list = hdf_utils.get_dot_configs_path(
            self.root, self.vendor, self.board)
        template_string = "LOSCFG_DRIVERS_HDF_${module_upper}_${driver_upper}=y\n"
        new_demo_config = Template(template_string).substitute(
            {"module_upper": self.module.upper(),
             "driver_upper": self.driver.upper()})

        device_enable = self.__get_enable_config()
        for dot_file in dot_file_list:
            file_lines_old = hdf_utils.read_file_lines(dot_file)
            if device_enable:
                file_lines = list(
                    filter(lambda x: hdf_utils.judge_enable_line(
                        enable_line=x, device_base=device_enable.split("=")[0]),
                                         file_lines_old))
                if device_enable not in file_lines:
                    file_lines.append(device_enable)
            else:
                file_lines = file_lines_old
            file_lines[-1] = file_lines[-1].strip() + "\n"
            if new_demo_config not in file_lines:
                file_lines.append(new_demo_config)
            hdf_utils.write_file_lines(dot_file, file_lines)
        file_path[self.module + "_dot_configs"] = dot_file_list
        return file_path

    def driver_create_info_format(self, config_file_json,
                                  config_item, file_path):
        kernel_type = config_file_json.get(self.kernel)
        if kernel_type is None:
            config_file_json[self.kernel] = {
                config_item.get("module_name"): {
                    'module_leve_config': {},
                    "driver_file_list": {
                        config_item.get("driver_name"):
                            config_item.get("driver_file_path")
                    }
                }
            }
            config_file_json[self.kernel][self.module]["module_leve_config"]\
                .update(file_path)
        else:
            model_type = kernel_type.get(config_item.get("module_name"))
            if model_type is None:
                temp = config_file_json.get(self.kernel)
                temp_module = config_item.get("module_name")
                temp[temp_module] = {
                    'module_leve_config': {},
                    "driver_file_list": {
                        config_item.get("driver_name"):
                            config_item.get("driver_file_path")
                    }
                }
                config_file_json.get(self.kernel).get(self.module).\
                    get("module_leve_config").update(file_path)
            else:
                temp = config_file_json.get(self.kernel).\
                    get(config_item.get("module_name")).get("driver_file_list")
                temp[config_item.get("driver_name")] = \
                    config_item.get("driver_file_path")

        return config_file_json

    def add_driver(self, *args_tuple):
        root, vendor, module, driver, board, kernel, device = args_tuple
        drv_converter = hdf_utils.WordsConverter(driver)
        source_file, head_path, include_name = self.create_model_file_name(
            root, vendor, module, driver, board, kernel, device)
        data_model = {
            'driver_lower_case': drv_converter.lower_case(),
            'driver_upper_camel_case': drv_converter.upper_camel_case(),
            'driver_lower_camel_case': drv_converter.lower_camel_case(),
            "include_file": include_name,
            'driver_upper_case': drv_converter.upper_case()
        }
        if os.path.exists(source_file):
            return True, source_file
        templates_dir = hdf_utils.get_templates_lite_dir()
        templates_model_dir = []
        for path, dir_name, _ in os.walk(templates_dir):
            if dir_name:
                templates_model_dir.extend(dir_name)
        templates_model_dir = list(filter(
            lambda model_dir: self.module in model_dir,
            templates_model_dir))
        target_template_path = list(map(
            lambda dir_name: os.path.join(templates_dir, dir_name),
            templates_model_dir))[0]
        templates_file_list = os.listdir(target_template_path)
        source_file_template = os.path.join(
            target_template_path,
            list(filter(lambda file_name: "source" in file_name,
                        templates_file_list))[0])
        self._template_fill(source_file_template, source_file, data_model)
        head_file_template = os.path.join(
            target_template_path,
            list(filter(lambda file_name: "head" in file_name,
                        templates_file_list))[0])
        self._template_fill(head_file_template, head_path, data_model)
        child_dir_list, operation_object = hdf_utils.ini_file_read_operation(
            model=module, node_name='file_dir')
        if device not in child_dir_list:
            child_dir_list.append(device)
            hdf_utils.ini_file_write_operation(
                module, operation_object, child_dir_list)
        return True, source_file

    def _file_gen_lite(self, template, source_file_path, model):
        templates_dir = hdf_utils.get_templates_lite_dir()
        template_path = os.path.join(templates_dir, template)
        self._template_fill(template_path, source_file_path, model)

    def _template_fill(self, template_path, output_path, data_model):
        if not os.path.exists(template_path):
            return
        raw_content = hdf_utils.read_file(template_path)
        contents = Template(raw_content).safe_substitute(data_model)
        hdf_utils.write_file(output_path, contents)

    def create_model_file_name(self, *args_tuple):
        root, vendor, module, driver, board, kernel, device = args_tuple
        drv_src_dir = hdf_utils.get_drv_src_dir(root, module)
        if device.strip():
            if module == "sensor":
                new_mkdir_path = os.path.join(drv_src_dir, 'chipset', device)
            else:
                new_mkdir_path = os.path.join(drv_src_dir, device)
            if not os.path.exists(new_mkdir_path):
                os.mkdir(new_mkdir_path)
            result_path_source = os.path.join(new_mkdir_path, '%s_%s_driver.c' % (device, driver))
            result_path_head = os.path.join(new_mkdir_path, '%s_%s_driver.h' % (device, driver))
            include_name = '%s_%s_driver.h' % (device, driver)
        else:
            if module == "sensor":
                new_mkdir_path = os.path.join(drv_src_dir, 'chipset', driver)
            else:
                new_mkdir_path = os.path.join(drv_src_dir, driver)
            if not os.path.exists(new_mkdir_path):
                os.mkdir(new_mkdir_path)
            result_path_source = os.path.join(new_mkdir_path, '%s_driver.c' % driver)
            result_path_head = os.path.join(new_mkdir_path, '%s_driver.h' % driver)
            include_name = '%s_driver.h' % driver
        return result_path_source, result_path_head, include_name

    def __get_enable_config(self):
        templates_dir = hdf_utils.get_templates_lite_dir()
        templates_model_dir = []
        for path, dir_name, _ in os.walk(templates_dir):
            if dir_name:
                templates_model_dir.extend(dir_name)
        templates_model_dir = list(filter(lambda model_dir: self.module in model_dir, templates_model_dir))
        config_file = [name for name in os.listdir(os.path.join(templates_dir, templates_model_dir[0])) if
                       name.endswith("ini")]
        if config_file:
            config_path = os.path.join(templates_dir, templates_model_dir[0], config_file[0])
            config = configparser.ConfigParser()
            config.read(config_path)
            section_list = config.options(section=self.kernel)
            if self.device in section_list:
                device_enable_config, _ = hdf_utils.ini_file_read_operation(
                    model=self.kernel, node_name=self.device, path=config_path)
            else:
                if self.kernel == "linux":
                    device_enable_config = ["CONFIG_DRIVERS_HDF_SENSOR_ACCEL=y\n"]
                else:
                    device_enable_config = ["LOSCFG_DRIVERS_HDF_SENSOR_ACCEL=y\n"]
        else:
            device_enable_config = [""]
        return device_enable_config[0]
