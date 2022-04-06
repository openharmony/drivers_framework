#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


from string import Template

import hdf_utils


def find_makefile_file_end_index(date_lines, model_name):
    file_end_flag = "ccflags-y"
    end_index = 0
    # INPUT_ROOT_DIR info
    model_dir_name = ("%s_ROOT_DIR" % model_name.upper())
    model_dir_value = ""

    for index, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.strip().startswith(file_end_flag):
            end_index = index
            break
        elif line.strip().startswith(model_dir_name):
            model_dir_value = line.split("=")[-1].strip()
        else:
            continue
    result_tuple = (end_index, model_dir_name, model_dir_value)
    return result_tuple


def linux_makefile_operation(path, driver_file_path, module, driver):
    makefile_gn_path = path
    date_lines = hdf_utils.read_file_lines(makefile_gn_path)
    source_file_path = driver_file_path.replace('\\', '/')
    result_tuple = find_makefile_file_end_index(date_lines, model_name=module)
    judge_result = judge_driver_config_exists(date_lines, driver_name=driver)
    if judge_result:
        return
    end_index, model_dir_name, model_dir_value = result_tuple

    first_line = "\nobj-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\\n"
    second_line = "              $(${model_name_upper}_ROOT_DIR)/${source_file_path}\n"
    makefile_add_template = first_line + second_line
    include_model_info = model_dir_value.split("model")[-1].strip('"')+"/"
    makefile_path_config = source_file_path.split(include_model_info)
    temp_handle = Template(makefile_add_template.replace("$(", "temp_flag"))
    d = {
        'model_name_upper': module.upper(),
        'driver_name_upper': driver.upper(),
        'source_file_path': makefile_path_config[-1].replace(".c", ".o")
    }
    new_line = temp_handle.substitute(d).replace("temp_flag", "$(")

    date_lines = date_lines[:end_index-1] + [new_line] + date_lines[end_index-1:]
    hdf_utils.write_file_lines(makefile_gn_path, date_lines)


def judge_driver_config_exists(date_lines, driver_name):
    for _, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.find(driver_name) != -1:
            return True
    return False