/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "audio_parse_test.h"
#include "audio_parse.h"
#include "devsvc_manager_clnt.h"

#define HDF_LOG_TAG audio_parse_test
#define TEST_PARSE_SERVICE_NAME "hdf_audio_codec_dev0"

int32_t AudioFillTestConfigData(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioFillConfigData(NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d]: AudioFillConfigData fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(TEST_PARSE_SERVICE_NAME);
    struct AudioConfigData configData;
    if (AudioFillConfigData(device, &configData) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d]: AudioFillConfigData fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}
