/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_dma_base_test.h"
#include "audio_platform_if.h"
#include "audio_dma_base.h"
#include "audio_driver_log.h"

#define HDF_LOG_TAG audio_dma_base_test

int32_t AudioDmaBufAllocTest(void)
{
    struct PlatformData data;

    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));

    if (AudioDmaBufAlloc(NULL, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaBufAlloc(&data, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaBufFreeTest(void)
{
    struct PlatformData data;

    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));

    if (AudioDmaBufFree(NULL, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaBufFree(&data, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaRequestChannelTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaRequestChannel(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaRequestChannel(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaConfigChannelTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaConfigChannel(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaConfigChannel(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaPrepTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaPrep(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaPrep(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaSubmitTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaSubmit(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaSubmit(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaPendingTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaPending(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaPending(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaPauseTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaPause(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaPause(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaResumeTest(void)
{
    struct PlatformData data;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaResume(NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaResume(&data) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioDmaPointerTest(void)
{
    struct PlatformData data;
    uint32_t pointer = 0;
    (void)memset_s(&data, sizeof(struct PlatformData), 0, sizeof(struct PlatformData));
    if (AudioDmaPointer(NULL, NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaPointer(&data, &pointer) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}
