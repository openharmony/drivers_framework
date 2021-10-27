/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "regulator_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"

#define HDF_LOG_TAG regulator_core

enum RegulatorDisableMode {
    NORMAL_DISABLE,
    DEFERRED_DISABLE,
    FORCE_DISABLE,
    MAX_DISABLE_MODE,
};

static int32_t RegulatorNormalDisable(struct RegulatorCntlr *cntlr);
static int32_t RegulatorDeferredDisable(struct RegulatorCntlr *cntlr);
static int32_t RegulatorForceDisable(struct RegulatorCntlr *cntlr);

struct RegulatorTree *RegulatorTreeGet(struct RegulatorCntlr *cntlr, const char *name)
{
    int i;

    for (i = 0; i < REGULATOR_NUM_MAX; i++) {
        if (strcmp(name, cntlr->rootTree[i].desc.name) == 0) {
            return &cntlr->rootTree[i];
        }
    }

    HDF_LOGE("can't get tree!,func:%s",__func__);
    return NULL;
}

void *RegulatorCntlrGet(const char *name)
{
    void *handle;
    struct RegulatorCntlr *cntlr;
    handle = (void *)DevSvcManagerClntGetService("HDF_PLATFORM_REGULATOR");
    if (handle == NULL) {
        HDF_LOGE("RegulatorGetByName: get handle fail!");
    }
    RegulatorGetPrivData((struct RegulatorCntlr *)handle);
    cntlr = (struct RegulatorCntlr *)handle;
    cntlr->tree = RegulatorTreeGet((struct RegulatorCntlr *)handle, name);

    return (void *)handle;
}

int32_t RegulatorCntlrAdd(struct RegulatorCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("RegulatorCntlrAdd: cntlr is NULL!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->device == NULL) {
        HDF_LOGE("RegulatorCntlrAdd: no device associated!");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = OsalSpinInit(&cntlr->spinLock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("RegulatorCntlrAdd: spinlock init fail!");
        return ret;
    }

    ret = OsalMutexInit(&cntlr->mutexLock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("RegulatorCntlrAdd: mutexlock init fail!");
        return ret;
    }

    cntlr->device->service = &cntlr->service;
    return HDF_SUCCESS;
}

void RegulatorCntlrRemove(struct RegulatorCntlr *cntlr)
{
    if (cntlr == NULL) {
        return;
    }

    if (cntlr->device == NULL) {
        HDF_LOGE("RegulatorCntlrRemove: cntlr is NULL!");
        return;
    }

    cntlr->device->service = NULL;
    (void)OsalMutexDestroy(&cntlr->mutexLock);
    (void)OsalSpinDestroy(&cntlr->spinLock);
}

void RegulatorGetPrivData(struct RegulatorCntlr *cntlr)
{
    if (cntlr == NULL || cntlr->ops == NULL) {
        HDF_LOGE("RegulatorGetPrivData: cntlr is NULL!");
        return;
    }
    if (cntlr->ops->getPriv != NULL) {
        cntlr->ops->getPriv(cntlr);
    }
}

void RegulatorReleasePriv(struct RegulatorCntlr *cntlr)
{
    if (cntlr == NULL || cntlr->ops == NULL) {
        HDF_LOGE("RegulatorReleasePriv: cntlr is NULL!");
        return;
    }
    if (cntlr->ops->releasePriv != NULL) {
        cntlr->ops->releasePriv(cntlr);
    }
}

int32_t RegulatorCntlrEnable(struct RegulatorCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->enable == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }
    
    ret = RegulatorCntlrIsEnabled(cntlr);
    if (ret != HDF_SUCCESS) {
        ret = RegulatorCntlrEnable((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName));
        if (ret != HDF_SUCCESS) {
            goto EXIT;
        }
    }

    ret = cntlr->ops->enable(cntlr);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    ret = RegulatorCntlrSetVoltage((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName),
                                    (cntlr->tree->initData.initInPutUv + cntlr->tree->supply->desc.outPutUv));
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    ret = RegulatorCntlrSetVoltage(cntlr, cntlr->tree->initData.initOutPutUv);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }
    cntlr->tree->desc.status = REGULATOR_STATUS_ON;
    cntlr->tree->supply->desc.enabledCount += 1;
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;

    EXIT:
    cntlr->tree->constraint.alwaysOn = false;
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
    return HDF_FAILURE;
}

static int32_t RegulatorNormalDisable(struct RegulatorCntlr *cntlr)
{
    int32_t ret;

    if (cntlr->tree->constraint.alwaysOn != true) {
        HDF_LOGE("device always on,func:%s",__func__);
        return HDF_FAILURE;
    }

    if (cntlr->tree->desc.enabledCount != 0) {
        HDF_LOGE("consumer is working,func:%s",__func__);
        return HDF_FAILURE;
    }

    ret = cntlr->ops->disable(cntlr);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }
    cntlr->tree->desc.outPutUv = 0;
    cntlr->tree->desc.status = REGULATOR_STATUS_OFF;
    ret = RegulatorCntlrSetVoltage((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName),
                                    (cntlr->tree->supply->desc.outPutUv - cntlr->tree->desc.inPutUv));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }
    cntlr->tree->supply->desc.enabledCount -= 1;
    cntlr->tree->constraint.alwaysOn = false;

    if (cntlr->tree->supply->desc.enabledCount == 0) {
        ret = RegulatorNormalDisable((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName));
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("supply disable failed,func:%s",__func__);
        }
    }

    return HDF_SUCCESS;
}

static int32_t RegulatorDeferredDisable(struct RegulatorCntlr *cntlr)
{
    int32_t ret;
    int i;

    if (cntlr->tree->constraint.alwaysOn != true) {
        HDF_LOGW("device always on,func:%s",__func__);
        return HDF_FAILURE;
    }

    if (cntlr->tree->desc.enabledCount != 0) {
        for (i = 0; i < cntlr->tree->constraint.maxMountQuantity; i++) {
            if (cntlr->tree->consumer[i]->constraint.alwaysOn != false) {
                HDF_LOGW("consumer device always on,func:%s",__func__);
                return HDF_FAILURE;
            }
        }
        OsalSleep(REGULATOR_SLEEP_TIME);
        ret = RegulatorForceDisable(cntlr);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("RegulatorForceDisable failed,func:%s",__func__);
            return HDF_FAILURE;
        }
        return HDF_SUCCESS;
    }

    ret = cntlr->ops->disable(cntlr);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }
    cntlr->tree->desc.outPutUv = 0;
    cntlr->tree->desc.status = REGULATOR_STATUS_OFF;
    ret = RegulatorCntlrSetVoltage((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName),
                                    (cntlr->tree->supply->desc.outPutUv - cntlr->tree->desc.inPutUv));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }
    cntlr->tree->supply->desc.enabledCount -= 1;
    cntlr->tree->constraint.alwaysOn = false;

    if (cntlr->tree->supply->desc.enabledCount == 0) {
        ret = RegulatorNormalDisable((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName));
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("supply disable failed,func:%s",__func__);
        }
    }

    return HDF_SUCCESS;
}

static int32_t RegulatorForceDisable(struct RegulatorCntlr *cntlr)
{
    int32_t ret;

    ret = cntlr->ops->disable(cntlr);
    if (ret != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    cntlr->tree->desc.outPutUv = 0;
    cntlr->tree->desc.status = REGULATOR_STATUS_OFF;
    ret = RegulatorCntlrSetVoltage((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName),
                                    (cntlr->tree->supply->desc.outPutUv - cntlr->tree->desc.inPutUv));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }
    cntlr->tree->supply->desc.enabledCount -= 1;
    cntlr->tree->constraint.alwaysOn = false;

    if (cntlr->tree->supply->desc.enabledCount == 0) {
        ret = RegulatorNormalDisable((struct RegulatorCntlr *)RegulatorCntlrGet(cntlr->tree->desc.supplyName));
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("supply disable failed,func:%s",__func__);
        }
    }

    return HDF_SUCCESS;
}

int32_t RegulatorCntlrDisable(struct RegulatorCntlr *cntlr, int32_t disableMode)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->disable == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    switch (disableMode)
    {
    case NORMAL_DISABLE:
        ret = RegulatorNormalDisable(cntlr);
        break;
    case DEFERRED_DISABLE:
        ret = RegulatorDeferredDisable(cntlr);
        break;
    case FORCE_DISABLE:
        ret = RegulatorForceDisable(cntlr);
        break;
    default:
        ret = HDF_FAILURE;
        break;
    }

    if (ret != HDF_SUCCESS) {
        (void)OsalMutexUnlock(&cntlr->mutexLock);
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;
}

int32_t RegulatorCntlrIsEnabled(struct RegulatorCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->isEnabled == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    if (cntlr->tree->constraint.alwaysOn == true) {
        cntlr->tree->desc.status = REGULATOR_STATUS_ON;
        return HDF_SUCCESS;
    }

    ret = cntlr->ops->isEnabled(cntlr);
    if (cntlr->tree->constraint.alwaysOn == false) {
        cntlr->tree->desc.status = REGULATOR_STATUS_OFF;
        (void)OsalMutexUnlock(&cntlr->mutexLock);
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }

    cntlr->tree->desc.status = REGULATOR_STATUS_ON;
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;
}

int32_t RegulatorCntlrSetVoltage(struct RegulatorCntlr *cntlr, int32_t voltage)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setVoltage == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = RegulatorCntlrIsEnabled(cntlr);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    if (voltage < cntlr->tree->desc.minOutPutUv || voltage > cntlr->tree->desc.maxOutPutUv) {
        goto EXIT;
    }

    ret = cntlr->ops->setVoltage(cntlr, voltage);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;

    EXIT:
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
    return HDF_FAILURE;
}

int32_t RegulatorCntlrGetVoltage(struct RegulatorCntlr *cntlr, int32_t *voltage)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getVoltage == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = cntlr->ops->getVoltage(cntlr, voltage);
    if (ret != HDF_SUCCESS) {
        (void)OsalMutexUnlock(&cntlr->mutexLock);
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;
}

int32_t RegulatorCntlrSetVoltageRange(struct RegulatorCntlr *cntlr, int32_t vmin, int32_t vmax)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setVoltageRange == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = RegulatorCntlrIsEnabled(cntlr);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    if (vmin < cntlr->tree->constraint.minOutPutUv || vmax > cntlr->tree->constraint.maxOutPutUv) {
        goto EXIT;
    }

    ret = cntlr->ops->setVoltageRange(cntlr, vmin, vmax);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;

    EXIT:
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
    return HDF_FAILURE;
}

int32_t RegulatorCntlrSetCurrent(struct RegulatorCntlr *cntlr, int32_t current)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setCurrent == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = RegulatorCntlrIsEnabled(cntlr);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    if (current < cntlr->tree->desc.minOutPutUa || current > cntlr->tree->desc.maxOutPutUa) {
        goto EXIT;
    }

    ret = cntlr->ops->setCurrent(cntlr, current);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;

    EXIT:
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
    return HDF_FAILURE;
}

int32_t RegulatorCntlrGetCurrent(struct RegulatorCntlr *cntlr, int32_t *current)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getCurrent == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = cntlr->ops->getCurrent(cntlr, current);
    if (ret != HDF_SUCCESS) {
        (void)OsalMutexUnlock(&cntlr->mutexLock);
        HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
        return HDF_FAILURE;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;
}

int32_t RegulatorCntlrSetCurrentRange(struct RegulatorCntlr *cntlr, int32_t cmin, int32_t cmax)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setCurrentRange == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = RegulatorCntlrIsEnabled(cntlr);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    if (cmin < cntlr->tree->constraint.minOutPutUa || cmax > cntlr->tree->constraint.maxOutPutUa) {
        goto EXIT;
    }

    ret = cntlr->ops->setVoltageRange(cntlr, cmin, cmax);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return HDF_SUCCESS;

    EXIT:
    (void)OsalMutexUnlock(&cntlr->mutexLock);
    HDF_LOGE("func:%s failed!,line:%d", __func__, __LINE__);
    return HDF_FAILURE;
}

int32_t RegulatorCntlrGetStatus(struct RegulatorCntlr *cntlr, int32_t *status)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("func:%s cntlr is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getStatus == NULL) {
        HDF_LOGE("func:%s ops is NULL!", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexLock(&cntlr->mutexLock) != HDF_SUCCESS) {
        HDF_LOGE("func:%s OsalMutexLock failed!", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = cntlr->ops->getStatus(cntlr, status);

    (void)OsalMutexUnlock(&cntlr->mutexLock);
    return ret;
}