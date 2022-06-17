/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ioservicebind_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include "hdf_base.h"
#include "hdf_io_service.h"
#include "parcel.h"

namespace OHOS {
bool IoserviceBindFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    Parcel parcel;
    parcel.WriteBuffer(data, size);
    auto servicename = parcel.ReadCString();
    struct HdfIoService *testServ = HdfIoServiceBind(servicename);
    if (testServ != nullptr) {
        result = true;
    }
    HdfIoServiceRecycle(testServ);
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::IoserviceBindFuzzTest(data, size);
    return HDF_SUCCESS;
}
