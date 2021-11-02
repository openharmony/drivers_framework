/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef REGULATOR_CORE_H
#define REGULATOR_CORE_H

#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_mutex.h"
#include "osal_spinlock.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define REGULATOR_NUM_MAX   50
#define REGULATOR_MOUNT_QUANTITY_MAX   10
#define REGULATOR_NAME_LEN 128
#define REGULATOR_SLEEP_TIME 500

struct RegulatorCntlr;
struct RegulatorMethod;

enum RegulatorStatus {
    REGULATOR_STATUS_ON,
    REGULATOR_STATUS_OFF,
};

struct RegulatorDesc {
    const char *name;
    const char *supplyName;
    uint32_t minOutPutUv;
    uint32_t maxOutPutUv;
    uint32_t minOutPutUa;
    uint32_t maxOutPutUa;
    uint32_t outPutUv;
    uint32_t outPutUa;
    uint32_t inPutUv;
    uint32_t inPutUa;
    uint32_t enabledCount;
    uint32_t status;
};

struct RegulatorConstraint {
    bool alwaysOn;
    uint32_t maxMountQuantity;
    uint32_t minOutPutUv;
    uint32_t maxOutPutUv;
    uint32_t minOutPutUa;
    uint32_t maxOutPutUa;
};

struct RegulatorInitdata {
    uint32_t initInPutUv;
    uint32_t initInPutUa;
    uint32_t initOutPutUv;
    uint32_t initOutPutUa;
};

struct RegulatorTree {
    struct RegulatorTree *supply;
    struct RegulatorDesc desc;
    struct RegulatorConstraint constraint;
    struct RegulatorInitdata initData;
    struct RegulatorTree *consumer[REGULATOR_MOUNT_QUANTITY_MAX];
};

struct RegulatorCntlr {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    struct RegulatorMethod *ops;
    OsalSpinlock spinLock;
    struct OsalMutex mutexLock;
    struct RegulatorTree *tree;
    struct RegulatorTree *rootTree;
    void *priv;
};

struct RegulatorMethod {
    void (*getPriv)(struct RegulatorCntlr *cntlr);
    void (*releasePriv)(struct RegulatorCntlr *cntlr);
    int32_t (*enable)(struct RegulatorCntlr *cntlr);
    int32_t (*disable)(struct RegulatorCntlr *cntlr);
    int32_t (*isEnabled)(struct RegulatorCntlr *cntlr);
    int32_t (*setVoltage)(struct RegulatorCntlr *cntlr, int32_t voltage);
    int32_t (*getVoltage)(struct RegulatorCntlr *cntlr, int32_t *voltage);
    int32_t (*setVoltageRange)(struct RegulatorCntlr *cntlr, int32_t vmin, int32_t vmax);
    int32_t (*setCurrent)(struct RegulatorCntlr *cntlr, int32_t current);
    int32_t (*getCurrent)(struct RegulatorCntlr *cntlr, int32_t *current);
    int32_t (*setCurrentRange)(struct RegulatorCntlr *cntlr, int32_t cmin, int32_t cmax);
    int32_t (*getStatus)(struct RegulatorCntlr *cntlr, int32_t *status);
};

int32_t RegulatorCntlrAdd(struct RegulatorCntlr *cntlr);

void RegulatorCntlrRemove(struct RegulatorCntlr *cntlr);

static inline struct RegulatorCntlr *RegulatorCntlrFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct RegulatorCntlr *)device->service;
}

static inline struct HdfDeviceObject *RegulatorCntlrToDevice(struct RegulatorCntlr *cntlr)
{
    return (cntlr == NULL) ? NULL : cntlr->device;
}

void *RegulatorCntlrGet(const char *name);
void RegulatorGetPrivData(struct RegulatorCntlr *cntlr);
void RegulatorReleasePriv(struct RegulatorCntlr *cntlr);
int32_t RegulatorCntlrEnable(struct RegulatorCntlr *cntlr);
int32_t RegulatorCntlrDisable(struct RegulatorCntlr *cntlr, int32_t disableMode);
int32_t RegulatorCntlrIsEnabled(struct RegulatorCntlr *cntlr);
int32_t RegulatorCntlrSetVoltage(struct RegulatorCntlr *cntlr, int32_t voltage);
int32_t RegulatorCntlrGetVoltage(struct RegulatorCntlr *cntlr, int32_t *voltage);
int32_t RegulatorCntlrSetVoltageRange(struct RegulatorCntlr *cntlr, int32_t vmax, int32_t vmin);
int32_t RegulatorCntlrSetCurrent(struct RegulatorCntlr *cntlr, int32_t current);
int32_t RegulatorCntlrGetCurrent(struct RegulatorCntlr *cntlr, int32_t *current);
int32_t RegulatorCntlrSetCurrentRange(struct RegulatorCntlr *cntlr, int32_t cmin, int32_t cmax);
int32_t RegulatorCntlrGetStatus(struct RegulatorCntlr *cntlr, int32_t *status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* REGULATOR_CORE_H */
