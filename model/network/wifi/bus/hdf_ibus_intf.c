/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "osal_mem.h"
#include "securec.h"
#include "wifi_inc.h"
#include "hdf_log.h"
#include "hdf_wlan_config.h"
#include "hdf_base.h"
#include "hdf_ibus_intf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct BusDev *HdfWlanCreateBusManager(const struct HdfConfigWlanBus *busConfig)
{
    struct BusDev *bus = NULL;
    if (busConfig == NULL) {
        return NULL;
    }
    if (busConfig->busType >= BUS_BUTT) {
        HDF_LOGE("%s:bus type %u not support!", __func__, busConfig->busType);
        return NULL;
    }
    
    bus = (struct BusDev *)OsalMemCalloc(sizeof(struct BusDev));
    if (bus == NULL) {
        return NULL;
    }

    if (HdfWlanBusAbsInit(bus, busConfig) != HDF_SUCCESS) {
        OsalMemFree(bus);
        return NULL;
    }
    return bus;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
