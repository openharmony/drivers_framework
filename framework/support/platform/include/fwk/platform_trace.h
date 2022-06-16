/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_TRACE_H
#define PLATFORM_TRACE_H
#include "hdf_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    PLATFORM_TRACE_UINT_PARAM_SIZE_1 = 1,
    PLATFORM_TRACE_UINT_PARAM_SIZE_2 = 2,
    PLATFORM_TRACE_UINT_PARAM_SIZE_3 = 3,
    PLATFORM_TRACE_UINT_PARAM_SIZE_4 = 4,
    PLATFORM_TRACE_UINT_PARAM_SIZE_MAX = 5
} PLATFORM_TRACE_UINT_PARAM_SIZE;

#define PLATFORM_TRACE_MODULE_MAX_NUM 255
/*
 * if you want to use the trace function, please add your trace definition of the moudule.
 * it can help you understand the meaning of the trace info.
 */
typedef enum {
    PLATFORM_TRACE_MODULE_I2S = 1,
    PLATFORM_TRACE_MODULE_I2C,
    PLATFORM_TRACE_MODULE_PWM,
    PLATFORM_TRACE_MODULE_TIMER,
    PLATFORM_TRACE_MODULE_UNITTEST,
    PLATFORM_TRACE_MODULE_MAX = PLATFORM_TRACE_MODULE_MAX_NUM,
} PLATFORM_TRACE_MODULE;

/*
 * if you want to use the trace function, please add your trace definition of the moudule function.
 * it can help you understand the meaning of the trace info.
 */
typedef enum {
    PLATFORM_TRACE_MODULE_I2S_FUN = PLATFORM_TRACE_MODULE_MAX_NUM + 1,
    PLATFORM_TRACE_MODULE_I2S_READ_DATA,
    PLATFORM_TRACE_MODULE_I2C_FUN = PLATFORM_TRACE_MODULE_I2S_FUN + 50,
    PLATFORM_TRACE_MODULE_PWM_FUN = PLATFORM_TRACE_MODULE_I2C_FUN + 50,
    PLATFORM_TRACE_MODULE_PWM_FUN_SET_CONFIG,
    PLATFORM_TRACE_MODULE_TIMER_FUN = PLATFORM_TRACE_MODULE_PWM_FUN + 50,
    PLATFORM_TRACE_MODULE_TIMER_FUN_SET,
    PLATFORM_TRACE_MODULE_UNITTEST_FUN = PLATFORM_TRACE_MODULE_TIMER_FUN + 50,
    PLATFORM_TRACE_MODULE_UNITTEST_FUN_TEST,
    PLATFORM_TRACE_MODULE_MAX_FUN = 5000,
} PLATFORM_TRACE_MODULE_FUN;

struct PlatformTraceModuleExplain {
    int moduleInfo;
    const char *moduleMean;
};

const char *PlatformTraceModuleExplainGet(PLATFORM_TRACE_MODULE traceModule);
const char *PlatformTraceFunExplainGet(PLATFORM_TRACE_MODULE_FUN traceFun);
const struct PlatformTraceModuleExplain *PlatformTraceModuleExplainGetAll(void);
int32_t PlatformTraceModuleExplainCount(void);

int32_t PlatformTraceStart(void);
int32_t PlatformTraceStop(void);
void PlatformTraceInfoDump(void);
void PlatformTraceReset(void);

void __attribute__((format(printf, 3, 4)))
PlatformTraceAddMsg(const char *module, const char *moduleFun, const char *fmt, ...);
void PlatformTraceAddUintMsg(int module, int moduleFun, uint infos[], uint8_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PLATFORM_TRACE_H */
