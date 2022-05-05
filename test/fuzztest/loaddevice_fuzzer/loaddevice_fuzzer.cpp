/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "loaddevice_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include "idevmgr_hdi.h"

using namespace OHOS::HDI::DeviceManager::V1_0;

namespace OHOS {
bool LoadDeviceFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    std::string serviceName((const char *)data);
    sptr<IDeviceManager> devicemanager = IDeviceManager::Get();
    if (!devicemanager->LoadDevice(serviceName)) {
        result = true;
    }
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::LoadDeviceFuzzTest(data, size);
    return 0;
}
