/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "platform_trace.h"
static struct PlatformTraceModuleExplain g_traceModuleExplain[] = {
    {PLATFORM_TRACE_MODULE_I2S,                "PLATFORM_TRACE_MODULE_I2S"               },
    {PLATFORM_TRACE_MODULE_I2C,                "PLATFORM_TRACE_MODULE_I2C"               },
    {PLATFORM_TRACE_MODULE_PWM,                "PLATFORM_TRACE_MODULE_PWM"               },
    {PLATFORM_TRACE_MODULE_TIMER,              "PLATFORM_TRACE_MODULE_TIMER"             },
    {PLATFORM_TRACE_MODULE_UNITTEST,           "PLATFORM_TRACE_MODULE_UNITTEST"          },
    {PLATFORM_TRACE_MODULE_MAX,                "PLATFORM_TRACE_MODULE_MAX"               },
    {PLATFORM_TRACE_MODULE_I2S_READ_DATA,      "PLATFORM_TRACE_MODULE_I2S_READ_DATA"     },
    {PLATFORM_TRACE_MODULE_PWM_FUN_SET_CONFIG, "PLATFORM_TRACE_MODULE_PWM_FUN_SET_CONFIG"},
    {PLATFORM_TRACE_MODULE_TIMER_FUN_SET,      "PLATFORM_TRACE_MODULE_TIMER_FUN_SET"     },
    {PLATFORM_TRACE_MODULE_UNITTEST_FUN_TEST,  "PLATFORM_TRACE_MODULE_UNITTEST_FUN_TEST" },
};

const char *PlatformTraceModuleExplainGet(PLATFORM_TRACE_MODULE traceModule)
{
    uint32_t i = 0;
    for (i = 0; i < sizeof(g_traceModuleExplain) / sizeof(g_traceModuleExplain[0]); i++) {
        if (traceModule == g_traceModuleExplain[i].moduleInfo) {
            return g_traceModuleExplain[i].moduleMean;
        }
    }
    return NULL;
}

const char *PlatformTraceFunExplainGet(PLATFORM_TRACE_MODULE_FUN traceFun)
{
    uint32_t i = 0;
    for (i = 0; i < sizeof(g_traceModuleExplain) / sizeof(g_traceModuleExplain[0]); i++) {
        if (traceFun == g_traceModuleExplain[i].moduleInfo) {
            return g_traceModuleExplain[i].moduleMean;
        }
    }
    return NULL;
}

const struct PlatformTraceModuleExplain *PlatformTraceModuleExplainGetAll(void)
{
    return g_traceModuleExplain;
}

int32_t PlatformTraceModuleExplainCount(void)
{
    return sizeof(g_traceModuleExplain) / sizeof(g_traceModuleExplain[0]);
}
