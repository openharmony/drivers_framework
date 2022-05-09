/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "platform_trace_test.h"
#include "hdf_log.h"
#include "platform_trace.h"

#define HDF_LOG_TAG platform_trace_test

#define TRACE_UNIT_INFO_TEST_SIZE 6
static uint g_infos[TRACE_UNIT_INFO_TEST_SIZE] = {100, 200, 300, 400, 500, 600};
static int32_t PlatformTraceSetUintptrInfoTest()
{
    int i = 0;
    int32_t ret = PlatformTraceStart();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("PlatformTraceSetUintptrInfoTest: PlatformTraceStart fail %d", ret);
        return ret;
    }

    for (i = 1; i <= TRACE_UNIT_INFO_TEST_SIZE; i++) {
        PlatformTraceAddUintMsg(PLATFORM_TRACE_MODULE_UNITTEST, PLATFORM_TRACE_MODULE_UNITTEST_FUN_TEST,
            g_infos, i);
    }
    PlatformTraceStop();
    PlatformTraceInfoDump();
    return HDF_SUCCESS;
}

static int32_t PlatformTraceFmtInfoTest()
{
    int32_t ret = PlatformTraceStart();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("PlatformTraceSetUintptrInfoTest: PlatformTraceStart fail %d", ret);
        return ret;
    }
    PlatformTraceAddMsg(
        "TraceTestModule", "TraceTestModuleFun", ":%d-%d-%s-%s", g_infos[0], g_infos[1], "testparam1", "testparam2");
    PlatformTraceStop();
    PlatformTraceInfoDump();
    return HDF_SUCCESS;
}

static int32_t PlatformTraceTestReliability()
{
    PlatformTraceAddUintMsg(PLATFORM_TRACE_MODULE_MAX, PLATFORM_TRACE_MODULE_MAX_FUN, NULL, 0);
    PlatformTraceAddMsg(NULL, "TraceTestModuleFun", ":%d-%d-%s-%s",
        g_infos[0], g_infos[1], "testparam1", "testparam2");

    return HDF_SUCCESS;
}

struct PlatformTraceTestEntry {
    int cmd;
    int32_t (*func)(void);
};

static struct PlatformTraceTestEntry g_entry[] = {
    {PLATFORM_TRACE_TEST_SET_UINT,    PlatformTraceSetUintptrInfoTest},
    {PLATFORM_TRACE_TEST_SET_FMT,     PlatformTraceFmtInfoTest       },
    {PLATFORM_TRACE_TEST_RELIABILITY, PlatformTraceTestReliability   },
};

int PlatformTraceTestExecute(int cmd)
{
#ifdef __LITEOS__
#ifndef LOSCFG_KERNEL_TRACE
    HDF_LOGE("PlatformTraceTestExecute: please open trace info");
    return HDF_SUCCESS;
#endif
#else
#ifndef CONFIG_EVENT_TRACING
    HDF_LOGE("PlatformTraceTestExecute: please open trace info");
    return HDF_SUCCESS;
#endif
#endif

    uint32_t i;
    int32_t ret = HDF_ERR_NOT_SUPPORT;
    struct PlatformTraceTestEntry *entry = NULL;

    if (cmd > PLATFORM_TRACE_TEST_CMD_MAX) {
        HDF_LOGE("PlatformTraceTestExecute: invalid cmd:%d", cmd);
        return HDF_ERR_NOT_SUPPORT;
    }

    for (i = 0; i < sizeof(g_entry) / sizeof(g_entry[0]); i++) {
        if (g_entry[i].cmd != cmd || g_entry[i].func == NULL) {
            continue;
        }
        entry = &g_entry[i];
        break;
    }

    if (entry == NULL) {
        HDF_LOGE("%s: no entry matched, cmd = %d", __func__, cmd);
        return HDF_ERR_NOT_SUPPORT;
    }

    ret = entry->func();
    HDF_LOGI("[PlatformTraceTestExecute][======cmd:%d====ret:%d======]", cmd, ret);
    return ret;
}
