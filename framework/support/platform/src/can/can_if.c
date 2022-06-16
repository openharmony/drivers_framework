/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can_if.h"
#include "can/can_client.h"
#include "can/can_core.h"

int32_t CanBusOpen(int32_t number, DevHandle *handle)
{
    int32_t ret;
    struct CanClient *client = NULL;

    ret = CanClientCreateByNumber(number, &client);
    if (ret == HDF_SUCCESS) {
        *handle = (DevHandle)client;
    }
    return ret;
}

void CanBusClose(DevHandle handle)
{
    CanClientDestroy((struct CanClient *)handle);
}

int32_t CanBusSendMsg(DevHandle handle, const struct CanMsg *msg)
{
    return CanClientWriteMsg((struct CanClient *)handle, msg);
}

int32_t CanBusReadMsg(DevHandle handle, struct CanMsg *msg, uint32_t tms)
{
    return CanClientReadMsg((struct CanClient *)handle, msg, tms);
}

int32_t CanBusAddFilter(DevHandle handle, const struct CanFilter *filter)
{
    return CanClientAddFilter((struct CanClient *)handle, filter);
}

int32_t CanBusDelFilter(DevHandle handle, const struct CanFilter *filter)
{
    return CanClientDelFilter((struct CanClient *)handle, filter);
}

int32_t CanBusSetCfg(DevHandle handle, const struct CanConfig *cfg)
{
    return CanClientSetCfg((struct CanClient *)handle, cfg);
}

int32_t CanBusGetCfg(DevHandle handle, struct CanConfig *cfg)
{
    return CanClientGetCfg((struct CanClient *)handle, cfg);
}

int32_t CanBusGetState(DevHandle handle)
{
    return CanClientGetState((struct CanClient *)handle);
}
