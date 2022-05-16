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
#include "hdf_io_service.h"

namespace OHOS {
bool IoserviceGroupRemoveServiceFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    struct HdfIoService *serv = HdfIoServiceBind((const char *)data);
    if (serv == nullptr) {
        return false;
    }
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    if (group == nullptr) {
        HdfIoServiceRecycle(serv);
        return false;
    }
    int ret = HdfIoServiceGroupAddService(group, serv);
    if (ret != HDF_SUCCESS) {
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

