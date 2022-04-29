/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can/can_mail.h"
#include "can/can_msg.h"
#include "hdf_dlist.h"
#include "hdf_log.h"
#include "osal_mem.h"

struct CanRxBox *CanRxBoxCreate()
{
    struct CanRxBox *rbox = NULL;

    rbox = (struct CanRxBox *)OsalMemCalloc(sizeof(*rbox));
    if (rbox == NULL) {
        HDF_LOGE("CanRxBoxCreate: malloc failed");
        return NULL;
    }

    rbox->queue = PlatformQueueCreate(NULL, "can_rbox", rbox);
    if (rbox->queue == NULL) {
        HDF_LOGE("CanRxBoxCreate: create rbox queue failed");
        OsalMemFree(rbox);
        return NULL;
    }

    DListHeadInit(&rbox->filters);
    (void)OsalSpinInit(&rbox->spin);
    return rbox;
}

void CanRxBoxDestroy(struct CanRxBox *rbox)
{
    if (rbox != NULL) {
        if (rbox->queue != NULL) {
            PlatformQueueDestroy(rbox->queue);
        }
        OsalSpinDestroy(&rbox->spin);
        OsalMemFree(rbox);
    }
}

static bool CanRxBoxMsgMatch(struct CanRxBox *, const struct CanMsg *);

int32_t CanRxBoxAddMsg(struct CanRxBox *rbox, struct CanMsg *cmsg)
{
    int32_t ret;
    struct PlatformMsg *pmsg = NULL;

    if (rbox == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (!CanRxBoxMsgMatch(rbox, cmsg)) {
        return HDF_ERR_NOT_SUPPORT;
    }

    pmsg = (struct PlatformMsg *)OsalMemCalloc(sizeof(*pmsg));
    if (pmsg == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }
    pmsg->data = cmsg;
    CanMsgGet(cmsg); // increase ref count before enqueue
    ret = PlatformQueueAddMsg(rbox->queue, pmsg);
    if (ret != HDF_SUCCESS) {
        CanMsgPut(cmsg); // decrase ref count if enqueue failed 
    }
    return ret;
}

int32_t CanRxBoxGetMsg(struct CanRxBox *rbox, struct CanMsg **cmsg, uint32_t tms)
{
    int32_t ret;
    struct PlatformMsg *pmsg = NULL;

    if (rbox == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = PlatformQueueGetMsg(rbox->queue, &pmsg, tms);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("CanRxBoxGetMsg: get platform msg failed:%d", ret);
        return ret;
    }

    *cmsg = (struct CanMsg *)pmsg->data;
    OsalMemFree(pmsg);
    return HDF_SUCCESS;
}

static bool CanFilterMatch(const struct CanFilter *filter, const struct CanMsg *cmsg)
{
    (void)filter;
    (void)cmsg;
    uint32_t mask;

    if (filter->rtrMask == 1 && filter->rtr != cmsg->rtr) {
        return false;
    }

    if (filter->ideMask == 1 && filter->ide != cmsg->ide) {
        return false;
    }

    mask = (cmsg->ide == 1) ? 0x7FF : 0x1FFFFFFF; // 11bits or 29bits ?
    mask &= filter->idMask;
    return ((cmsg->id & mask) == (filter->id & mask));
}

static bool CanRxBoxMsgMatch(struct CanRxBox *rbox, const struct CanMsg *cmsg)
{
    bool match = true;
    struct CanFilterNode *cfNode = NULL;

    if (rbox == NULL || cmsg == NULL) {
        return false;
    }

    CanRxBoxLock(rbox);
    DLIST_FOR_EACH_ENTRY(cfNode, &rbox->filters, struct CanFilterNode, node) {
        if (CanFilterMatch(cfNode->filter, cmsg)) {
            match = true;
            break;
        }
        match = false;
    }
    CanRxBoxUnlock(rbox);
    return match;
}

int32_t CanRxBoxAddFilter(struct CanRxBox *rbox, const struct CanFilter *filter)
{
    struct CanFilterNode *cfNode = NULL;

    if (rbox == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (filter == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    cfNode = (struct CanFilterNode *)OsalMemCalloc(sizeof(*cfNode));
    if (cfNode == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }
    cfNode->filter = filter;
    cfNode->active = true;
    CanRxBoxLock(rbox);
    DListInsertTail(&cfNode->node, &rbox->filters);
    CanRxBoxUnlock(rbox);
    return HDF_SUCCESS;
}

int32_t CanRxBoxDelFilter(struct CanRxBox *rbox, const struct CanFilter *filter)
{
    struct CanFilterNode *cfNode = NULL;
    struct CanFilterNode *tmp = NULL;

    if (rbox == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (filter == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    CanRxBoxLock(rbox);
    DLIST_FOR_EACH_ENTRY_SAFE(cfNode, tmp, &rbox->filters, struct CanFilterNode, node) {
        if (cfNode->filter == filter) {
            DListRemove(&cfNode->node);
            CanRxBoxUnlock(rbox);
            OsalMemFree(cfNode);
            return HDF_SUCCESS;
        }
    }
    return HDF_ERR_NOT_SUPPORT;
}
