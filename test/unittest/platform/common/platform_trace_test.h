/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_TRACE_TEST_H
#define PLATFORM_TRACE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

enum PlatformTraceTestCmd {
    PLATFORM_TRACE_TEST_SET_UINT = 0,
    PLATFORM_TRACE_TEST_SET_FMT = 1,
    PLATFORM_TRACE_TEST_RELIABILITY = 2,
    PLATFORM_TRACE_TEST_CMD_MAX,
};

int PlatformTraceTestExecute(int cmd);
void PlatformTraceTestExecuteAll(void);

#ifdef __cplusplus
}
#endif
#endif /* PLATFORM_TRACE_TEST_H */
