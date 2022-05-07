/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "registerservicestatuslistener_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <hdi_base.h>
#include <iremote_broker.h>
#include <refbase.h>
#include "iservmgr_hdi.h"
#include "iservstat_listener_hdi.h"

using namespace OHOS::HDI::ServiceManager::V1_0;

namespace OHOS {
namespace HDI {
namespace ServiceManager {
namespace V1_0 {
void RegisterServStatListenerFuzzer::OnReceive(const ServiceStatus &status)
{
    (void)status;
}
} // namespace V1_0
} // namespace ServiceManager
} // namespace HDI
} // namespace OHOS

namespace OHOS {
bool RegisterServiceStatusListenerFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    sptr<IServiceManager> servmanager = IServiceManager::Get();
    sptr<IServStatListener> servstatlistener = new RegisterServStatListenerFuzzer();
    if (!servmanager->RegisterServiceStatusListener(servstatlistener, reinterpret_cast<uintptr_t>(data))) {
        result = true;
    }
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::RegisterServiceStatusListenerFuzzTest(data, size);
    return 0;
}
