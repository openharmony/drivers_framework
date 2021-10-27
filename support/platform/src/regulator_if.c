/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "regulator_if.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "securec.h"
#include "regulator_core.h"

DevHandle RegulatorGet(const char *name)
{
    return (DevHandle)RegulatorCntlrGet(name);
}

void RegulatorPut(DevHandle handle)
{
    if (handle == NULL) {
        return;
    }
    RegulatorReleasePriv((struct RegulatorCntlr *)handle);
    (void)handle;
}

int32_t RegulatorEnable(DevHandle handle)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrEnable((struct RegulatorCntlr *)handle);
}

int32_t RegulatorDisable(DevHandle handle, int32_t disableMode)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrDisable((struct RegulatorCntlr *)handle, disableMode);
}

int32_t RegulatorIsEnabled(DevHandle handle)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrIsEnabled((struct RegulatorCntlr *)handle);
}

int32_t RegulatorSetVoltage(DevHandle handle, int32_t voltage)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrSetVoltage((struct RegulatorCntlr *)handle, voltage);
}

int32_t RegulatorGetVoltage(DevHandle handle, int32_t *voltage)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrGetVoltage((struct RegulatorCntlr *)handle, voltage);
}

int32_t RegulatorSetVoltageRange(DevHandle handle, int32_t vmin, int32_t vmax)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrSetVoltageRange((struct RegulatorCntlr *)handle, vmin, vmax);
}

int32_t RegulatorSetCurrent(DevHandle handle, int32_t current)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrSetCurrent((struct RegulatorCntlr *)handle, current);
}

int32_t RegulatorGetCurrent(DevHandle handle, int32_t *current)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrGetCurrent((struct RegulatorCntlr *)handle, current);
}

int32_t RegulatorSetCurrentRange(DevHandle handle, int32_t cmin, int32_t cmax)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrSetCurrentRange((struct RegulatorCntlr *)handle, cmin, cmax);
}

int32_t RegulatorGetStatus(DevHandle handle, int32_t *status)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return RegulatorCntlrGetStatus((struct RegulatorCntlr *)handle, status);
}