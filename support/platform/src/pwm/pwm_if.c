/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "osal_mem.h"
#include "pwm_core.h"
#include "securec.h"

#define HDF_LOG_TAG pwm_if
#define PWM_NAME_LEN 32

static void *PwmGetDevByNum(uint32_t num)
{
    int32_t ret;
    char *name = NULL;
    void *pwm = NULL;

    name = (char *)OsalMemCalloc(PWM_NAME_LEN + 1);
    if (name == NULL) {
        return NULL;
    }
    ret = snprintf_s(name, PWM_NAME_LEN + 1, PWM_NAME_LEN, "HDF_PLATFORM_PWM_%u", num);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        OsalMemFree(name);
        return NULL;
    }
    pwm = (void *)DevSvcManagerClntGetService(name);
    OsalMemFree(name);
    return pwm;
}

DevHandle PwmOpen(uint32_t num)
{
    int32_t ret;
    void *pwm = PwmGetDevByNum(num);

    if (pwm == NULL) {
        HDF_LOGE("%s: dev is null", __func__);
        return NULL;
    }

    ret = PwmDeviceGet((struct PwmDev *)pwm);
    if(ret != HDF_SUCCESS) {
        HDF_LOGE("%s: PwmDeviceGet failed, ret: %d", __func__, ret);
        return NULL;
    }
    return (DevHandle)pwm;
}

void PwmClose(DevHandle handle)
{
    if (handle == NULL) {
        HDF_LOGE("%s: dev is null", __func__);
        return;
    }

    PwmDevicePut((struct PwmDev *)handle);
}

int32_t PwmSetPeriod(DevHandle handle, uint32_t period)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.period;
    config.period = period;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. period: %d -> %d.", __func__, curValue, config.period);
    }
    return ret;
}

int32_t PwmSetDuty(DevHandle handle, uint32_t duty)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.duty;
    config.duty = duty;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. duty: %d -> %d.", __func__, curValue, config.duty);
    }
    return ret;
}

int32_t PwmSetPolarity(DevHandle handle, uint8_t polarity)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.polarity;
    config.polarity = polarity;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. polarity: %d -> %d.", __func__, curValue, config.polarity);
    }
    return ret;
}

int32_t PwmEnable(DevHandle handle)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.status;
    config.status = PWM_ENABLE_STATUS;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. enable: %d -> %d.", __func__, curValue, config.status);
    }
    return ret;
}

int32_t PwmDisable(DevHandle handle)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.status;
    config.status = PWM_DISABLE_STATUS;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. enable: %d -> %d.", __func__, curValue, config.status);
    }
    return ret;
}

int32_t PwmSetConfig(DevHandle handle, struct PwmConfig *config)
{
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (handle == NULL) {
        HDF_LOGE("%s: handle is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (config == NULL) {
        HDF_LOGE("%s: config is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = PwmDeviceSetConfig(handle, config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: PwmSetConfig failed, ret: %d", __func__, ret);
    }

    HDF_LOGI("%s: success.", __func__);
    return HDF_SUCCESS;
}

int32_t PwmGetConfig(DevHandle handle, struct PwmConfig *config)
{
    struct PwmDev *pwm = NULL;

    HDF_LOGI("%s: enter.", __func__);
    if (handle == NULL) {
        HDF_LOGE("%s: handle is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (config == NULL) {
        HDF_LOGE("%s: config is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    pwm = (struct PwmDev *)handle;
    *config = pwm->cfg;
    HDF_LOGI("%s: success.", __func__);

    return HDF_SUCCESS;
}