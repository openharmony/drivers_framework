/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_DUMPER_TEST_H
#define PLATFORM_DUMPER_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

enum PlatformDumperTestCmd {
    PLAT_DUMPER_TEST_ADD_UINT8 = 0,
    PLAT_DUMPER_TEST_ADD_UINT16,
    PLAT_DUMPER_TEST_ADD_UINT32,
    PLAT_DUMPER_TEST_ADD_UINT64,
    PLAT_DUMPER_TEST_ADD_INT8,
    PLAT_DUMPER_TEST_ADD_INT16,
    PLAT_DUMPER_TEST_ADD_INT32,
    PLAT_DUMPER_TEST_ADD_INT64,
    PLAT_DUMPER_TEST_ADD_FLOAT,
    PLAT_DUMPER_TEST_ADD_DOUBLE,
    PLAT_DUMPER_TEST_ADD_CHAR,
    PLAT_DUMPER_TEST_ADD_STRING,
    PLAT_DUMPER_TEST_ADD_REGISTER,
    PLAT_DUMPER_TEST_ADD_ARRAY_DATA,
    PLAT_DUMPER_TEST_SET_OPS,
    PLAT_DUMPER_TEST_MULTI_THREAD,
    PLAT_DUMPER_TEST_DEL_DATA,
    PLAT_DUMPER_TEST_RELIABILITY,
    PLAT_DUMPER_TEST_CMD_MAX,
};

#define PLAT_DUMPER_TEST_STACK_SIZE   (1024 * 100)
#define PLAT_DUMPER_TEST_WAIT_TIMES   200
#define PLAT_DUMPER_TEST_WAIT_TIMEOUT 20

int PlatformDumperTestExecute(int cmd);
void PlatformDumperTestExecuteAll(void);

#ifdef __cplusplus
}
#endif
#endif /* PLATFORM_DUMPER_TEST_H */
