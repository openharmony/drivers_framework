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
from ..liteos.gn_file_add_config import analyze_parent_path


def find_makefile_file_end_index(date_lines, model_name):
    file_end_flag = "ccflags-y"
    end_index = 0
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


def audio_linux_makefile_operation(path, args_tuple):
    source_path, head_path, module, driver, root, devices = args_tuple
    makefile_gn_path = path
    date_lines = hdf_utils.read_file_lines(makefile_gn_path)
    judge_result = judge_driver_config_exists(date_lines, driver_name=driver)
    if judge_result:
        return
    if len(source_path) > 1:
        sources_line = ""
        obj_first_line = "\nobj-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\"+"\n"
        temp_line = "\t\t\t\t$(${file_parent_path})/${source_path}.o"
        for source in source_path:
            temp_handle = Template(temp_line.replace("$(", "temp_flag"))
            temp_dict = analyze_parent_path(
                date_lines, source, "", devices, root)
            temp_dict['source_path'] = temp_dict['source_path'].strip(".c")
            if source == source_path[-1]:
                sources_line += temp_handle.substitute(temp_dict).replace("temp_flag", "$(") + "\n"
            else:
                sources_line += temp_handle.substitute(temp_dict).replace("temp_flag", "$(") + " \\" + "\n"
        build_resource = obj_first_line + sources_line
    else:
        build_resource = "LOCAL_SRCS += $(${file_parent_path})/${source_path}\n"
        for source in source_path:
            temp_handle = Template(build_resource.replace("$(", "temp_flag"))
            temp_dict = analyze_parent_path(
                date_lines, source, "", devices, root)
            build_resource = temp_handle.substitute(temp_dict).replace("temp_flag", "$(")
    head_line = ""
    ccflags_first_line = "\nccflags-$(CONFIG_DRIVERS_HDF_${model_name_upper}_${driver_name_upper}) += \\" + "\n"
    temp_line = "\t\t\t\t-I$(srctree)/$(${file_parent_path})/${head_path}"
    for head_file in head_path:
        temp_handle = Template(temp_line.replace("$(", "temp_flag"))
        temp_dict = analyze_parent_path(
            date_lines, "", head_file, devices, root)
        if head_file == head_path[-1]:
            head_line += temp_handle.substitute(temp_dict).replace("temp_flag", "$(") + "\n"
        else:
            head_line += temp_handle.substitute(temp_dict).replace("temp_flag", "$(") + " \\" + "\n"
    build_head = ccflags_first_line + head_line
    makefile_add_template = build_resource + build_head
    temp_handle = Template(makefile_add_template.replace("$(", "temp_flag"))
    temp_replace = {
        'model_name_upper': module.upper(),
        'driver_name_upper': driver.upper(),
    }
    new_line = temp_handle.substitute(temp_replace).replace("temp_flag", "$(")
    date_lines = date_lines + [new_line]
    hdf_utils.write_file_lines(makefile_gn_path, date_lines)


def judge_driver_config_exists(date_lines, driver_name):
    for _, line in enumerate(date_lines):
        if line.startswith("#"):
            continue
        elif line.find(driver_name) != -1:
            return True
    return False


def linux_makefile_operation(path, driver_file_path, head_path, module, driver):
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
    third_line = "ccflags-y += -I$(srctree)/drivers/hdf/framework/model/${head_file_path}\n"
    makefile_add_template = first_line + second_line + third_line
    include_model_info = model_dir_value.split("model")[-1].strip('"')+"/"
    makefile_path_config = source_file_path.split(include_model_info)
    temp_handle = Template(makefile_add_template.replace("$(", "temp_flag"))
    temp_replace = {
        'model_name_upper': module.upper(),
        'driver_name_upper': driver.upper(),
        'source_file_path': makefile_path_config[-1].replace(".c", ".o"),
        'head_file_path': '/'.join(head_path.split("model")[-1].strip(os.path.sep).split(os.path.sep)[:-1])
    }
    new_line = temp_handle.substitute(temp_replace).replace("temp_flag", "$(")
    endif_status = False
    for index, v in enumerate(date_lines[::-1]):
        if v.strip().startswith("endif"):
            endif_status = True
            end_line_index = len(date_lines) - index - 1
            date_lines = date_lines[:end_line_index] + [new_line] + [date_lines[end_line_index]]
            break
    if not endif_status:
        date_lines = date_lines + [new_line]
    hdf_utils.write_file_lines(makefile_gn_path, date_lines)
