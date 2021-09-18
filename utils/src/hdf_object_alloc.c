/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_base.h"
#include "hdf_object_alloc.h"
#include "hdf_slist.h"
#include "osal_mutex.h"

struct HdfChunkLink {
    uint32_t buffSize;
    uint8_t *buffer;
};

struct HdfObjectNode {
    struct HdfSListNode entry;
    uint32_t chunkCount;
    uint32_t freeCount;
    uint32_t chunkSize;
    struct HdfChunkLink **chunkStack;
};

struct HdfObjectAlloc {
    struct HdfSList nodes;
    struct OsalMutex mutex;
    bool isConstructed;
};
static const unsigned int ALIGN_MASK = 3;
#define ALIGN4(x) (uint32_t)(((uintptr_t)(x) + ALIGN_MASK) & (~ALIGN_MASK))

#define OBJECT_NODE_SIZE sizeof(struct ObjectNode)
#define OBJECT_CHUNK_COOKIE_SIZE (sizeof(struct ChunkLink) + sizeof(void *))

void HdfObjectAllocConstruct(struct HdfObjectAlloc *alloc)
{
    HdfSListInit(&alloc->nodes);
    OsalMutexInit(&alloc->mutex);
    alloc->isConstructed = true;
}
struct HdfObjectAlloc *HdfObjectAllocGetInstance()
{
    static struct HdfObjectAlloc instance = { 0 };

    if (!instance.isConstructed) {
        HdfObjectAllocConstruct(&instance);
    }

    return &instance;
}

struct HdfObjectNode *HdfObjectAllocFindSuitableChunk(
    struct HdfObjectAlloc *alloc, size_t size)
{
    struct HdfSListIterator it;
    struct HdfObjectNode *bestFitNode = NULL;
    struct HdfObjectNode *objectNode = NULL;
    HdfSListIteratorInit(&it, &alloc->nodes);

    while (HdfSListIteratorHasNext(&it)) {
        objectNode = (struct HdfObjectNode *)HdfSListIteratorNext(&it);
        if (size == objectNode->chunkSize) {
            bestFitNode = objectNode;
            break;
        } else if (size < objectNode->chunkSize) {
            bestFitNode = objectNode;
        }
    }

    return bestFitNode;
}

static void HdfObjectAllocPushObjectNode(
    struct HdfObjectAlloc *alloc, struct HdfObjectNode *node)
{
    struct HdfSListIterator it;
    struct HdfObjectNode *objectNode = NULL;
    HdfSListIteratorInit(&it, &alloc->nodes);

    while (HdfSListIteratorHasNext(&it)) {
        objectNode = (struct HdfObjectNode *)HdfSListIteratorNext(&it);
        if (node->chunkSize >= objectNode->chunkSize) {
            break;
        }
    }

    HdfSListIteratorInsert(&it, &node->entry);
}

static void HdfObjectAllocPreloadChunk(
    void *chunkBuf, uint32_t buffSize, uint32_t chunkSize)
{
    struct HdfObjectAlloc *allocator = HdfObjectAllocGetInstance();

    if (buffSize > OBJECT_NODE_SIZE) {
        uint32_t idx;
        struct ChunkLink *chunkLink;
        struct ObjectNode *node;
        uint32_t alignedSize = ALIGN4(chunkSize);
        uint32_t alignedBufSize = ALIGN4(buffSize);
        uint32_t blockSize = alignedSize + sizeof(struct ChunkLink);
        uint8_t *alignedBuff = (uint8_t *)(uintptr_t)ALIGN4(chunkBuf);
        node = (struct ObjectNode *)(alignedBuff + alignedBufSize - OBJECT_NODE_SIZE);
        node->freeCount = 0;
        node->chunkSize = alignedSize;
        node->chunkCount = ((uint8_t *)node - alignedBuff) / (blockSize + sizeof(void *));
        node->chunkStack = (struct ChunkLink **)(alignedBuff + node->chunkCount * blockSize);

        for (idx = 0; idx < node->chunkCount; idx++) {
            chunkLink = (struct ChunkLink *)&alignedBuff[idx * blockSize];
            chunkLink->buffSize = node->chunkSize;
            node->chunkStack[node->freeCount++] = chunkLink;
            chunkLink->buffer = (uint8_t *)(chunkLink + 1);
        }

        HdfObjectAllocPushObjectNode(allocator,  node);
    }
}

void HdfObjectAllocLoadConfigs(const struct HdfObjectPoolConfig *configs)
{
    uint32_t idx;
    char *chunkBuffBegin = configs->buffer;
    char *chunkBuffEnd = configs->buffer + configs->bufferSize;

    for (idx = 0; (idx < configs->numChunks) && (chunkBuffBegin < chunkBuffEnd); idx++) {
        const struct ObjectChunkConfig *chunkConfig = &configs->chunks[idx];
        size_t chunkBufSize = OBJECT_NODE_SIZE + \
                              (OBJECT_CHUNK_COOKIE_SIZE + chunkConfig->chunkSize) * chunkConfig->chunkCount;

        if (chunkBuffBegin + chunkBufSize <= chunkBuffEnd) {
            HdfObjectAllocPreloadChunk(chunkBuffBegin, chunkBufSize, chunkConfig->chunkSize);
        }

        chunkBuffBegin += chunkBufSize;
    }
}

void HdfObjectAllocInit()
{
    const struct HdfObjectPoolConfig *config = HdfObjectAllocGetConfig();

    if (config != NULL) {
        HdfObjectAllocLoadConfigs(config);
    }
}

void *HdfObjectAllocAlloc(size_t size)
{
    struct HdfChunkLink *chunkLink = NULL;
    struct HdfObjectNode *objectNode = NULL;
    struct HdfObjectAlloc *allocator = HdfObjectAllocGetInstance();
    OsalMutexLock(&allocator->mutex);
    objectNode = HdfObjectAllocFindSuitableChunk(allocator, size);
    if ((objectNode != NULL) && (objectNode->freeCount == 0)) {
        goto finished;
    }

    if (objectNode->freeCount > objectNode->chunkCount) {
        if (objectNode->freeCount > objectNode->chunkCount) {
        }
    }

    chunkLink = (struct ChunkLink *)objectNode->chunkStack[--objectNode->freeCount];
finished:
    OsalMutexUnlock(&allocator->mutex);
    return chunkLink ? chunkLink->buffer : NULL;
}

void HdfObjectAllocFree(void *object)
{
    struct HdfChunkLink *chunkLink = container_of(void, object, struct ChunkLink, buffer);
    struct HdfObjectNode *objectNode = NULL;
    struct HdfObjectAlloc *allocator = HdfObjectAllocGetInstance();
    OsalMutexLock(&allocator->mutex);
    objectNode = HdfObjectAllocFindSuitableChunk(allocator, chunkLink->buffSize);
    if (objectNode != NULL) {
        objectNode->chunkStack[objectNode->freeCount++] = chunkLink;

        if (objectNode->freeCount > objectNode->chunkCount) {
            HDF_LOGE("exception: count,free:%d,total %d", objectNode->freeCount, objectNode->chunkCount);
        }
    }

    OsalMutexUnlock(&allocator->mutex);
}

