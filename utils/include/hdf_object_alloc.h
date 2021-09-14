/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OBJECT_ALLOC_H
#define OBJECT_ALLOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HdfObjectChunkConfig {
    uint32_t chunkSize;
    uint32_t chunkCount;
};

struct HdfObjectPoolConfig {
    char *buffer;
    uint32_t bufferSize;
    uint32_t numChunks;
    const struct HdfObjectChunkConfig *chunks;
};

void *HdfObjectAllocAlloc(size_t size);

void HdfObjectAllocFree(void *object);

const struct HdfObjectPoolConfig *ObjectAllocGetConfig(void);
void HdfObjectAllocInit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BLOCK_BUFFER_H */

