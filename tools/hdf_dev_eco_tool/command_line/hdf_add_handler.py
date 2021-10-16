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
import json
import platform
from string import Template

from .hdf_command_handler_base import HdfCommandHandlerBase
from .hdf_command_error_code import CommandErrorCode
from .hdf_device_info_hcs import HdfDeviceInfoHcsFile
from .hdf_vendor_build_file import HdfVendorBuildFile
from .hdf_vendor_kconfig_file import HdfVendorKconfigFile
from .hdf_vendor_mk_file import HdfVendorMkFile
from .hdf_driver_config_file import HdfDriverConfigFile
from hdf_tool_settings import HdfToolSettings
from hdf_tool_exception import HdfToolException
from .hdf_vendor_makefile import HdfVendorMakeFile
from .hdf_defconfig_patch import HdfDefconfigAndPatch
import hdf_utils


class HdfAddHandler(HdfCommandHandlerBase):
    def __init__(self, args):
        super(HdfAddHandler, self).__init__()
        self.cmd = 'add'
        self.handlers = {
            'vendor': self._add_vendor_handler,
            'module': self._add_module_handler,
            'driver': self._add_driver_handler,
            'config': self._add_config_handler,
        }
        self.parser.add_argument("--action_type",
                                 help=' '.join(self.handlers.keys()),
                                 required=True)
        self.parser.add_argument("--root_dir", required=True)       # 路径
        self.parser.add_argument("--vendor_name")                   # 厂商
        self.parser.add_argument("--module_name")                   # 模块名
        self.parser.add_argument("--driver_name")                   # 驱动名称
        self.parser.add_argument("--board_name")                    # 板子名称
        self.parser.add_argument("--kernel_name")                   # 内核名称
        self.args = self.parser.parse_args(args)

    @staticmethod
    def _render(template_path, output_path, data_model):
        if not os.path.exists(template_path):
            return
        raw_content = hdf_utils.read_file(template_path)
        contents = Template(raw_content).safe_substitute(data_model)
        hdf_utils.write_file(output_path, contents)

    def _file_gen_lite(self, template, out_dir, filename, model):
        templates_dir = hdf_utils.get_templates_lite_dir()
        template_path = os.path.join(templates_dir, template)
        file_path = os.path.join(out_dir, filename)
        self._render(template_path, file_path, model)

    def _add_vendor_handler(self):
        self.check_arg_raise_if_not_exist("vendor_name")
        root, vendor, _, _, board = self.get_args()
        target_dir = hdf_utils.get_vendor_hdf_dir(root, vendor)
        if os.path.exists(target_dir):
            raise HdfToolException(
                "%s already exists" %
                target_dir, CommandErrorCode.TARGET_ALREADY_EXIST)
        os.makedirs(target_dir)
        self._file_gen_lite('hdf_vendor_kconfig.template', target_dir,
                            'Kconfig', {})
        board_parent_path = HdfToolSettings().get_board_parent_path(board)
        if not board_parent_path:
            board_parent_path = 'vendor/hisilicon'
        data_model = {
            "board_parent_path": board_parent_path
        }
        self._file_gen_lite('hdf_vendor_mk.template', target_dir,
                            'hdf_vendor.mk', data_model)

    def _add_module_handler(self):
        self.check_arg_raise_if_not_exist("vendor_name")
        self.check_arg_raise_if_not_exist("module_name")
        self.check_arg_raise_if_not_exist("kernel_name")
        self.check_arg_raise_if_not_exist("board_name")
        self.check_arg_raise_if_not_exist("driver_name")

        args_tuple = self.get_args()
        root, vendor, module, driver, board, kernel = args_tuple
        converter = hdf_utils.WordsConverter(self.args.module_name)
        driver_name_converter = hdf_utils.WordsConverter(self.args.driver_name)
        framework_hdf = hdf_utils.get_vendor_hdf_dir_framework(root)
        if not os.path.exists(framework_hdf):
            raise HdfToolException(
                ' framework model path  "%s" not exist' %
                framework_hdf, CommandErrorCode.TARGET_NOT_EXIST)
        # 在 framework 目录下创建对应的 module 的文件目录用于存放 .c 驱动文件
        framework_drv_root_dir = hdf_utils.get_drv_root_dir(
            root, vendor, module)
        if os.path.exists(framework_drv_root_dir):
            raise HdfToolException('module "%s" already exist' % module,
                                   CommandErrorCode.TARGET_ALREADY_EXIST)
        os.makedirs(framework_drv_root_dir)

        # 创建 .c 模板驱动
        state, driver_file_path = self._add_driver_handler(*args_tuple)
        if not state:
            raise HdfToolException(
                'create drivers file fail  "%s" ' %
                driver_file_path.split("\\")[-1])
        adapter_hdf = hdf_utils.get_vendor_hdf_dir_adapter(root, kernel)
        if not os.path.exists(adapter_hdf):
            raise HdfToolException(
                ' adapter model path  "%s" not exist' %
                adapter_hdf, CommandErrorCode.TARGET_NOT_EXIST)

        # 创建 adapter 路径下的 module 文件夹
        adapter_model_path = os.path.join(adapter_hdf, 'model', module)
        if not os.path.exists(adapter_model_path):
            os.makedirs(adapter_model_path)

        data_model = {
            "module_upper_case": converter.upper_case(),
            "module_lower_case": converter.lower_case(),
            "driver_file_name": ("%s_driver.c" %
                                 driver_name_converter.lower_case()),
            "driver_name": driver_name_converter.lower_case()
        }
        # 创建 adapter 下的 module中的三个文件
        if kernel == 'liteos':
            file_path, model_level_config_file_path = \
                self._add_module_handler_liteos(
                    framework_hdf, adapter_model_path,
                    data_model, converter, *args_tuple)
        elif kernel == "linux":
            file_path, model_level_config_file_path = \
                self._add_module_handler_linux(
                    framework_hdf, adapter_model_path,
                    data_model, *args_tuple)
        else:
            file_path = {}
            model_level_config_file_path = {}
        config_item = {
            'module_name': module,
            'module_path': file_path,
            'driver_name': "%s_driver.c" % driver,
            'driver_file_path': driver_file_path,
            'enabled': True
        }
        config_file_out = {
            'module_name': module,
            'module_path': file_path,
            'driver_name': driver_file_path.split("\\")[-1],
            'driver_file_path': driver_file_path,
            'module_level_config_path': model_level_config_file_path
        }
        config_file = hdf_utils.read_file(
            os.path.join('resources', 'create_model.config'))
        config_file_json = json.loads(config_file)
        config_file_json[module] = config_file_out
        if platform.system() == "Windows":
            config_file_replace = json.dumps(config_file_json, indent=4).\
                replace(root.replace('\\', '\\\\') + '\\\\', "")
            hdf_utils.write_file(
                os.path.join('resources', 'create_model.config'),
                config_file_replace.replace('\\\\', '/'))
        if platform.system() == "Linux":
            config_file_replace = json.dumps(config_file_json, indent=4).\
                replace(root + '/', "")
            hdf_utils.write_file(
                os.path.join('resources', 'create_model.config'),
                config_file_replace)
        return json.dumps(config_item)

    def _add_module_handler_liteos(self, framework_hdf, adapter_model_path,
                                   data_model, converter,  *args_tuple):
        root, vendor, module, driver, board, kernel = args_tuple
        liteos_file_path = {}
        liteos_level_config_file_path = {}
        liteos_file_name = ['BUILD.gn', 'Kconfig', 'Makefile']
        template_path = "/".join([framework_hdf] + ["tools",
                    "hdf_dev_eco_tool", "resources", "templates", "lite"])
        for file_name in liteos_file_name:
            for i in os.listdir(template_path):
                if i.find(file_name.split(".")[0]) > 0:
                    out_path = os.path.join(adapter_model_path, file_name)
                    self._render(os.path.join(template_path, i),
                                 out_path, data_model)
                    liteos_file_path[file_name] = out_path
        # 修改 liteos 下的 Kconfig 文件
        vendor_k = HdfVendorKconfigFile(root, vendor, kernel, path="")
        vendor_k_path = vendor_k.add_module([module, 'Kconfig'])
        liteos_level_config_file_path[module+"_Kconfig"] = vendor_k_path

        # 修改 liteos 下的 hdf_lite.mk 文件
        vendor_mk = HdfVendorMkFile(root, vendor)
        vendor_mk_path = vendor_mk.add_module(module)
        liteos_level_config_file_path[module + "_hdf_lite"] = vendor_mk_path

        # 修改 liteos 下的 Build.gn 文件
        vendor_gn = HdfVendorBuildFile(root, vendor)
        vendor_gn_path = vendor_gn.add_module(module)
        liteos_level_config_file_path[module + "Build"] = vendor_gn_path

        # 修改  vendor/hisilicon/hispark_taurus/hdf_config/
        # device_info 下的 device_info.hcs 文件
        device_info = HdfDeviceInfoHcsFile(
            root, vendor, module, board, driver, path="")
        hcs_file_path = device_info.add_model_hcs_file_config()
        liteos_file_path["devices_info.hcs"] = hcs_file_path

        # 修改 dot_configs 的配置文件
        dot_file_list = hdf_utils.get_dot_configs_path(root, vendor, board)
        template_string = "LOSCFG_DRIVERS_HDF_${module_upper_case}=y\n"
        new_demo_config = Template(template_string).substitute(
            {"module_upper_case": converter.upper_case()})
        for dot_file in dot_file_list:
            file_lines = hdf_utils.read_file_lines(dot_file)
            file_lines[-1] = file_lines[-1].strip() + "\n"
            if new_demo_config != file_lines[-1]:
                file_lines.append(new_demo_config)
                hdf_utils.write_file_lines(dot_file, file_lines)
        liteos_level_config_file_path[module + "_dot_configs"] = dot_file_list
        return liteos_file_path, liteos_level_config_file_path

    def _add_module_handler_linux(self, framework_hdf, adapter_model_path,
                                  data_model, *args_tuple):
        root, vendor, module, driver, board, kernel = args_tuple
        linux_file_path = {}
        linux_level_config_file_path = {}
        linux_file_name = ['Kconfig', 'Makefile']
        template_path = "/".join([framework_hdf]
                                 + ["tools", "hdf_dev_eco_tool",
                                    "resources", "templates", "lite"])
        for file_name in linux_file_name:
            for i in hdf_utils.template_filename_filtrate(
                    template_path, kernel):
                if i.find(file_name.split(".")[0]) > 0:
                    out_path = os.path.join(adapter_model_path, file_name)
                    self._render(os.path.join(template_path, i),
                                 out_path, data_model)
                    linux_file_path[file_name] = out_path
        # 修改 linux 下的 Kconfig 文件
        vendor_k = HdfVendorKconfigFile(root, vendor, kernel, path="")
        vendor_k_path = vendor_k.add_module([module, 'Kconfig'])
        linux_level_config_file_path[module + "_Kconfig"] = vendor_k_path

        # 修改 linux 下的 Makefile 文件
        vendor_mk = HdfVendorMakeFile(root, vendor, kernel, path='')
        vendor_mk_path = vendor_mk.add_module(data_model)
        linux_level_config_file_path[module + "_Makefile"] = vendor_mk_path

        # 修改  vendor/hisilicon/hispark_taurus_linux/
        # hdf_config/device_info 下的 device_info.hcs 文件
        device_info = HdfDeviceInfoHcsFile(
            root, vendor, module, board, driver, path="")
        hcs_file_path = device_info.add_model_hcs_file_config()
        linux_file_path["devices_info.hcs"] = hcs_file_path

        # 修改 dot_configs 的配置文件
        template_string = "CONFIG_DRIVERS_HDF_${module_upper_case}=y\n"
        new_demo_config = Template(template_string).substitute(data_model)
        defconfig_patch = HdfDefconfigAndPatch(
            root, vendor, kernel, board,
            data_model, new_demo_config)

        config_path = defconfig_patch.get_config_config()
        files = []
        patch_list = defconfig_patch.add_module(config_path, files=files)
        config_path = defconfig_patch.get_config_patch()
        files1 = []
        defconfig_list = defconfig_patch.add_module(config_path, files=files1)
        linux_level_config_file_path[module + "_dot_configs"] = \
            list(set(patch_list + defconfig_list))
        return linux_file_path, linux_level_config_file_path

    def _add_driver_handler(self, *args_tuple):
        root, vendor, module, driver, board, kernel = args_tuple
        drv_converter = hdf_utils.WordsConverter(self.args.driver_name)
        drv_src_dir = hdf_utils.get_drv_src_dir(root, vendor, module)
        if os.path.exists(os.path.join(drv_src_dir, '%s_driver.c' % driver)):
            raise HdfToolException(
                'driver "%s" already exist' %
                driver, CommandErrorCode.TARGET_ALREADY_EXIST)
        data_model = {
            'driver_lower_case': drv_converter.lower_case(),
            'driver_upper_camel_case': drv_converter.upper_camel_case(),
            'driver_lower_camel_case': drv_converter.lower_camel_case(),
            'driver_upper_case': drv_converter.upper_case()
        }
        self._file_gen_lite('hdf_driver.c.template', drv_src_dir,
                            '%s_driver.c' % driver, data_model)
        driver_file_path = os.path.join(
            drv_src_dir, '%s_driver.c' % driver)
        return True, driver_file_path

    def _add_config_handler(self):
        self.check_arg_raise_if_not_exist("module_name")
        self.check_arg_raise_if_not_exist("driver_name")
        self.check_arg_raise_if_not_exist("board_name")
        root, _, module, driver, board = self.get_args()
        drv_config = HdfDriverConfigFile(root, board, module, driver)
        drv_config.create_driver()
        return drv_config.get_drv_config_path()


