/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "platform_queue.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_mutex.h"
#include "osal_thread.h"
#include "osal_time.h"
#include "platform_errno.h"
#include "platform_log.h"

#define PLAT_QUEUE_THREAD_STAK 20000
#define PLAT_QUEUE_DEPTH_MAX   32

static void PlatformQueueDoDestroy(struct PlatformQueue *queue)
{
    (void)OsalThreadDestroy(&queue->thread);
    (void)OsalSemDestroy(&queue->sem);
    (void)OsalSpinDestroy(&queue->spin);
    OsalMemFree(queue);
}

static int32_t PlatformQueueNextMsg(struct PlatformQueue *queue, struct PlatformMsg **msg)
{
    int32_t ret;

    (void)OsalSpinLock(&queue->spin);
    if (DListIsEmpty(&queue->msgs)) {
        ret = HDF_PLT_ERR_NO_DATA;
    } else {
        *msg = DLIST_FIRST_ENTRY(&queue->msgs, struct PlatformMsg, node);
        DListRemove(&((*msg)->node));
        queue->depth--;
        ret = HDF_SUCCESS;
    }
    (void)OsalSpinUnlock(&queue->spin);

    return ret;
}

static int32_t PlatformQueueWorker(void *data)
{
    int32_t ret;
    struct PlatformQueue *queue = (struct PlatformQueue *)data;
    struct PlatformMsg *msg = NULL;

    while (true) {
        /* wait envent */
        ret = OsalSemWait(&queue->sem, HDF_WAIT_FOREVER);
        if (ret != HDF_SUCCESS) {
            continue;
        }

        if (!queue->start) {
            queue->exited = true;
            break;
        }

        (void)PlatformQueueNextMsg(queue, &msg);
        /* message process */
        if (msg != NULL && queue->handle != NULL) {
            (void)(queue->handle(queue, msg));
            msg = NULL;
        }
    }
    return HDF_SUCCESS;
}

struct PlatformQueue *PlatformQueueCreate(PlatformMsgHandle handle, const char *name, void *data)
{
    struct PlatformQueue *queue = NULL;

    queue = (struct PlatformQueue *)OsalMemCalloc(sizeof(*queue));
    if (queue == NULL) {
        PLAT_LOGE("PlatformQueueCreate: alloc queue fail!");
        return NULL;
    }

    (void)OsalSpinInit(&queue->spin);
    (void)OsalSemInit(&queue->sem, 0);
    DListHeadInit(&queue->msgs);
    queue->name = (name == NULL) ? "PlatformQueue" : name;
    queue->handle = handle;
    queue->depth = 0;
    queue->depthMax = PLAT_QUEUE_DEPTH_MAX;
    queue->data = data;
    queue->start = false;
    queue->exited = false;
    return queue;
}

int32_t PlatformQueueStart(struct PlatformQueue *queue)
{
    int32_t ret;
    struct OsalThreadParam cfg;

    if (queue == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = OsalThreadCreate(&queue->thread, (OsalThreadEntry)PlatformQueueWorker, (void *)queue);
    (void)PlatformQueueWorker;
    ret = HDF_SUCCESS;
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("PlatformQueueStart: create thread fail!");
        return ret;
    }

    cfg.name = (char *)queue->name;
    cfg.priority = OSAL_THREAD_PRI_HIGHEST;
    cfg.stackSize = PLAT_QUEUE_THREAD_STAK;
    ret = OsalThreadStart(&queue->thread, &cfg);
    (void)cfg;
    ret = HDF_SUCCESS;
    if (ret != HDF_SUCCESS) {
        OsalThreadDestroy(&queue->thread);
        PLAT_LOGE("PlatformQueueStart: start thread fail:%d", ret);
        return ret;
    }
    queue->start = true;

    return HDF_SUCCESS;
}

void PlatformQueueDestroy(struct PlatformQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    if (queue->start) {
        queue->start = false;
        (void)OsalSemPost(&queue->sem);
        while (!queue->exited) {
            OsalMSleep(1);
        }
        PlatformQueueDoDestroy(queue);
    } else {
        PlatformQueueDoDestroy(queue);
    }
}

int32_t PlatformQueueAddMsg(struct PlatformQueue *queue, struct PlatformMsg *msg)
{
    if (queue == NULL || msg == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    DListHeadInit(&msg->node);
    msg->error = HDF_SUCCESS;
    (void)OsalSpinLock(&queue->spin);
    if (queue->depth >= queue->depthMax) {
        (void)OsalSpinUnlock(&queue->spin);
        HDF_LOGE("PlatformQueueAddMsg: queue(%s) full!", queue->name);
        return HDF_PLT_OUT_OF_RSC;
    }
    DListInsertTail(&msg->node, &queue->msgs);
    queue->depth++;
    (void)OsalSpinUnlock(&queue->spin);
    /* notify the worker thread */
    (void)OsalSemPost(&queue->sem);
    return HDF_SUCCESS;
}

int32_t PlatformQueueGetMsg(struct PlatformQueue *queue, struct PlatformMsg **msg, uint32_t tms)
{
    int32_t ret;

    if (queue == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (msg == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    ret = PlatformQueueNextMsg(queue, msg);
    if (ret == HDF_SUCCESS) {
        (void)OsalSemWait(&queue->sem, HDF_WAIT_FOREVER); // consume the semaphore after get
        return ret;
    }

    if (tms == 0) {
        return HDF_PLT_ERR_NO_DATA;
    }

    ret = OsalSemWait(&queue->sem, tms);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    return PlatformQueueNextMsg(queue, msg);
}

