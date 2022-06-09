/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_DUMPER_H
#define PLATFORM_DUMPER_H

#include "hdf_dlist.h"
#include "osal_sem.h"
#include "osal_spinlock.h"
#include "osal_thread.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum PlatformDumperDataType {
    PLATFORM_DUMPER_UINT8 = 0,
    PLATFORM_DUMPER_UINT16,
    PLATFORM_DUMPER_UINT32,
    PLATFORM_DUMPER_UINT64,
    PLATFORM_DUMPER_INT8,
    PLATFORM_DUMPER_INT16,
    PLATFORM_DUMPER_INT32,
    PLATFORM_DUMPER_INT64,
    PLATFORM_DUMPER_FLOAT,
    PLATFORM_DUMPER_DOUBLE,
    PLATFORM_DUMPER_CHAR,
    PLATFORM_DUMPER_STRING,
    PLATFORM_DUMPER_REGISTERL,
    PLATFORM_DUMPER_REGISTERW,
    PLATFORM_DUMPER_REGISTERB,
    PLATFORM_DUMPER_MAX,
};

struct PlatformDumperMethod {
    void (*dumperCfgInfo)(void);
    void (*dumperStatusInfo)(void);
    void (*dumperRegisterInfo)(void);
    void (*dumperStatisInfo)(void);
};

struct PlatformDumperData {
    const char *name; // name len < 32
    enum PlatformDumperDataType type;
    void *paddr;
};

struct PlatformDumper;

struct PlatformDumper *PlatformDumperCreate(const char *name);
void PlatformDumperDestroy(struct PlatformDumper *dumper);
int32_t PlatformDumperDump(struct PlatformDumper *dumper);
int32_t PlatformDumperAddData(struct PlatformDumper *dumper, const struct PlatformDumperData *data);
int32_t PlatformDumperAddDatas(struct PlatformDumper *dumper, struct PlatformDumperData datas[], int size);
int32_t PlatformDumperSetMethod(struct PlatformDumper *dumper, struct PlatformDumperMethod *ops);
int32_t PlatformDumperDelData(struct PlatformDumper *dumper, const char *name, enum PlatformDumperDataType type);
int32_t PlatformDumperClearDatas(struct PlatformDumper *dumper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PLATFORM_DUMPER_H */
