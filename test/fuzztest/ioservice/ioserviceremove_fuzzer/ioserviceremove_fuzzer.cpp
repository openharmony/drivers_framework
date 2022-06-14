/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ioserviceremove_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_io_service.h"
#include "parcel.h"

namespace OHOS {
bool IoserviceGroupRemoveServiceFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    Parcel parcel;
    parcel.WriteBuffer(data, size);
    auto servicename = parcel.ReadCString();
    struct HdfIoService *serv = HdfIoServiceBind(servicename);
    if (serv == nullptr) {
        HDF_LOGE("%{public}s: HdfIoServiceBind failed!", __func__);
        return false;
    }
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    if (group == nullptr) {
        HDF_LOGE("%{public}s: HdfIoServiceGroupObtain failed!", __func__);
        HdfIoServiceRecycle(serv);
        return false;
    }
    int ret = HdfIoServiceGroupAddService(group, serv);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: HdfIoServiceGroupAddService failed!", __func__);
        HdfIoServiceGroupRecycle(group);
        HdfIoServiceRecycle(serv);
        return false;
    }
    HdfIoServiceGroupRemoveService(group, serv);
    if (group == nullptr) {
        result = true;
    }
    HdfIoServiceGroupRecycle(group);
    HdfIoServiceRecycle(serv);
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::IoserviceGroupRemoveServiceFuzzTest(data, size);
    return HDF_SUCCESS;
}

