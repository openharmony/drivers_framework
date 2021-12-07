/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "pwm_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG pwm_core

int32_t PwmDeviceGet(struct PwmDev *pwm)
{
    int32_t ret;

    if (pwm == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    (void)OsalSpinLock(&(pwm->lock));
    if (pwm->busy) {
        (void)OsalSpinUnlock(&(pwm->lock));
        HDF_LOGE("%s: pwm%u is busy", __func__, pwm->num);
        return HDF_ERR_DEVICE_BUSY;
    }
    if (pwm->method != NULL && pwm->method->open != NULL) {
        ret = pwm->method->open(pwm);
        if (ret != HDF_SUCCESS) {
            (void)OsalSpinUnlock(&(pwm->lock));
            HDF_LOGE("%s: open failed, ret %d", __func__, ret);
            return HDF_FAILURE;
        }
    }

    pwm->busy = true;
    (void)OsalSpinUnlock(&(pwm->lock));
    return HDF_SUCCESS;
}

void PwmDevicePut(struct PwmDev *pwm)
{
    int32_t ret;
    
    if (pwm == NULL) {
        return;
    }

    if (pwm->method != NULL && pwm->method->close != NULL) {
        ret = pwm->method->close(pwm);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: close failed, ret %d", __func__, ret);
            return;
        }
    }
    (void)OsalSpinLock(&(pwm->lock));
    pwm->busy = false;
    (void)OsalSpinUnlock(&(pwm->lock));
}

int32_t PwmDeviceSetConfig(struct PwmDev *pwm, struct PwmConfig *config)
{
    int32_t ret;
    
    if (pwm == NULL || config == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (memcmp(config, &(pwm->cfg), sizeof(*config)) == 0) {
        HDF_LOGE("%s: do not need to set config", __func__);
        return HDF_SUCCESS;
    }
    if (pwm->method == NULL || pwm->method->setConfig == NULL) {
        HDF_LOGE("%s: setConfig is not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    HDF_LOGI("%s: set PwmConfig: number %u, period %u, duty %u, polarity %u, enable %u.", __func__,
        config->number, config->period, config->duty, config->polarity, config->status);
    ret = pwm->method->setConfig(pwm, config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: failed, ret %d", __func__, ret);
        return ret;
    }
    pwm->cfg = *config;

    return HDF_SUCCESS;
}

int32_t PwmSetPriv(struct PwmDev *pwm, void *priv)
{
    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    pwm->priv = priv;
    return HDF_SUCCESS;
}

void *PwmGetPriv(struct PwmDev *pwm)
{
    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return NULL;
    }
    return pwm->priv;
}

int32_t PwmDeviceAdd(struct HdfDeviceObject *obj, struct PwmDev *pwm)
{
    if (obj == NULL || pwm == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (pwm->method == NULL || pwm->method->setConfig == NULL) {
        HDF_LOGE("%s: setConfig is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (OsalSpinInit(&(pwm->lock)) != HDF_SUCCESS) {
        HDF_LOGE("%s: init spinlock fail", __func__);
        return HDF_FAILURE;
    }
    pwm->device = obj;
    obj->service = &(pwm->service);
    return HDF_SUCCESS;
}

void PwmDeviceRemove(struct HdfDeviceObject *obj, struct PwmDev *pwm)
{
    if (obj == NULL || pwm == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return;
    }
    (void)OsalSpinDestroy(&(pwm->lock));
    pwm->device = NULL;
    obj->service = NULL;
}
