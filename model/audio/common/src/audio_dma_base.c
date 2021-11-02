#include "audio_platform_if.h"

int32_t AudioDmaBufAlloc(struct PlatformData *data, enum AudioStreamType streamType)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaBufAlloc != NULL) {
        return data->ops->DmaBufAlloc(data, streamType);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaBufFree(struct PlatformData *data, enum AudioStreamType streamType)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaBufFree != NULL) {
        return data->ops->DmaBufFree(data, streamType);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaRequestChannel(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaConfigChannel != NULL) {
        return data->ops->DmaRequestChannel(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaConfigChannel(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaConfigChannel != NULL) {
        return data->ops->DmaConfigChannel(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaPrep(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaPrep != NULL) {
        return data->ops->DmaPrep(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaSubmit(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaSubmit != NULL) {
        return data->ops->DmaSubmit(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaPending(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaPending != NULL) {
        return data->ops->DmaPending(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaPause(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaPause != NULL) {
        return data->ops->DmaPause(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaResume(struct PlatformData *data)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaResume != NULL) {
        return data->ops->DmaResume(data);
    }
    return HDF_FAILURE;
}

int32_t AudioDmaPointer(struct PlatformData *data, uint32_t *pointer)
{
    if (data != NULL && data->ops != NULL && data->ops->DmaPointer != NULL) {
        return data->ops->DmaPointer(data, pointer);
    }
    return HDF_FAILURE;
}


