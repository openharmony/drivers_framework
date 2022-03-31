#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.


from string import Template

import hdf_utils


def find_build_file_end_index(date_lines, model_name):
    state = False
    end_index = 0
    frameworks_model_name = "FRAMEWORKS_%s_ROOT" % (model_name.upper())
    frameworks_model_value = ''
    for index, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.find("hdf_driver") != -1:
            state = True
            continue
        elif line.startswith("}") and state:
            end_index = index
            state = False
        elif line.strip().startswith(frameworks_model_name):
            frameworks_model_value = line.split("=")[-1].strip()
        else:
            continue
    result_tuple = (end_index, frameworks_model_name, frameworks_model_value)
    return result_tuple


def build_file_operation(path, driver_file_path, module, driver):

    build_gn_path = path
    date_lines = hdf_utils.read_file_lines(build_gn_path)
    source_file_path = driver_file_path.replace('\\', '/')
    result_tuple = find_build_file_end_index(date_lines, model_name=module)
    judge_result = judge_driver_config_exists(date_lines, driver_name=driver)
    if judge_result:
        return
    end_index, frameworks_name, frameworks_value = result_tuple
    
    first_line = "\n  if (defined(LOSCFG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper})) {\n"
    second_line = '    sources += [ "$FRAMEWORKS_${model_name_upper}_ROOT/${source_file_path}" ]\n'
    third_line = "  }\n"
    build_add_template = first_line + second_line + third_line
    include_model_info = frameworks_value.split("model")[-1].strip('"')+"/"
    build_gn_path_config = source_file_path.split(include_model_info)
    temp_handle = Template(build_add_template.replace("$FRAMEWORKS", "FRAMEWORKS"))
    d = {
        'model_name_upper': module.upper(),
        'driver_name_upper': driver.upper(),
        'source_file_path': build_gn_path_config[-1]
    }
    new_line = temp_handle.substitute(d).replace("FRAMEWORKS", "$FRAMEWORKS")

    date_lines = date_lines[:end_index] + [new_line] + date_lines[end_index:]
    hdf_utils.write_file_lines(build_gn_path, date_lines)


def judge_driver_config_exists(date_lines, driver_name):
    for _, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.find(driver_name) != -1:
            return True
    return False
