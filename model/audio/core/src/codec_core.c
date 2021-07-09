/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codec_core.h"

#define HDF_LOG_TAG codec_core

int32_t CodecDeviceReadReg(struct CodecDevice *codec, uint32_t reg, uint32_t *val)
{
    unsigned long acodecVir;
    struct VirtualAddress *virtualAdd = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry");

    if ((codec == NULL) || (codec->device == NULL) || (val == NULL)) {
        AUDIO_DRIVER_LOG_ERR("input param codec or codec->device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    virtualAdd = (struct VirtualAddress *)codec->device->priv;
    if (virtualAdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("virtualAdd is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    acodecVir = virtualAdd->acodecVir;
    *val = OSAL_READL((void *)(acodecVir + reg));

    AUDIO_DRIVER_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t CodecDeviceWriteReg(struct CodecDevice *codec, uint32_t reg, uint32_t value)
{
    unsigned long acodecVir;
    struct VirtualAddress *virtualAdd = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry");

    if ((codec == NULL) || (codec->device == NULL)) {
        AUDIO_DRIVER_LOG_ERR("param  codec or codec->device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    virtualAdd = (struct VirtualAddress *)codec->device->priv;
    if (virtualAdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("virtualAdd is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    acodecVir = virtualAdd->acodecVir;
    OSAL_WRITEL(value, (void *)(acodecVir + reg));

    AUDIO_DRIVER_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t AiaoDeviceReadReg(struct CodecDevice *codec, uint32_t reg, uint32_t *val)
{
    unsigned long aiaoVir;
    struct VirtualAddress *virtualAdd = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry");

    if ((codec == NULL) || (codec->device == NULL) || (val == NULL)) {
        AUDIO_DRIVER_LOG_ERR("codec or codec->device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    virtualAdd = (struct VirtualAddress *)codec->device->priv;
    if (virtualAdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("virtualAdd is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    aiaoVir = virtualAdd->aiaoVir;
    *val = OSAL_READL((void *)(aiaoVir + reg));

    AUDIO_DRIVER_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t AiaoDeviceWriteReg(struct CodecDevice *codec, uint32_t reg, uint32_t value)
{
    unsigned long aiaoVir;
    struct VirtualAddress *virtualAdd = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry");

    if ((codec == NULL) || (codec->device == NULL)) {
        AUDIO_DRIVER_LOG_ERR("codec or codec->device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    virtualAdd = (struct VirtualAddress *)codec->device->priv;
    if (virtualAdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("virtualAdd is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    aiaoVir = virtualAdd->aiaoVir;
    OSAL_WRITEL(value, (void *)(aiaoVir + reg));

    AUDIO_DRIVER_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t CodecGetServiceName(struct HdfDeviceObject *device, const char **drvCodecName)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input device para is nullptr.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("node instance is nullptr.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("from resouce get drsOps fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", drvCodecName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read codecServiceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

int32_t CodecGetDaiName(struct HdfDeviceObject *device, const char **drvDaiName)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("drs node is NULL.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("drs ops failed!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "codecDaiName", drvDaiName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read codecDaiName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}
