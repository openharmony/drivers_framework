#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


import os
from string import Template

import hdf_utils
from .mk_file_add_config import judge_driver_config_exists


def get_template_info(template_path):
    template_path = os.path.join(template_path,
                                 'driver_add_kconfig_config.template')
    return hdf_utils.read_file(template_path)


def kconfig_file_operation(path, module, driver, template_path):
    config_add_config = get_template_info(template_path)
    kconfig_gn_path = path
    date_lines = hdf_utils.read_file_lines(kconfig_gn_path)
    judge_result = judge_driver_config_exists(date_lines, driver_name=driver)
    if judge_result:
        return
    temp_handle = Template(config_add_config)
    temp_replace = {
        "model_name_upper": module.upper(),
        "model_name_lower": module.lower(),
        "driver_name_upper": driver.upper(),
        "driver_name_lower": driver.lower()
    }
    new_line = temp_handle.substitute(temp_replace)
    if module == "display":
        replace_str = "depends on DRIVERS_HDF_%s"
        old_str = replace_str % module.upper()
        new_str = replace_str % "disp".upper()
        new_line = new_line.replace(old_str, new_str)

    date_lines = date_lines + [new_line]
    hdf_utils.write_file_lines(kconfig_gn_path, date_lines)