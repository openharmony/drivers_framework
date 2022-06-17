/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can/can_msg.h"
#include "hdf_dlist.h"
#include "osal_mem.h"

struct CanMsgHolder {
    struct HdfSRef ref;
    struct CanMsg cmsg;
};

struct CanClient {
    struct CanCntlr *cntlr;
    struct CanRxBox *rxBox; // receive message box
};

static void CanMsgHolderOnFirstGet(struct HdfSRef *sref)
{
    (void)sref;
}

static void CanMsgDoRecycle(struct CanMsg *msg)
{
    struct CanMsgHolder *msgExt = NULL;

    if (msg == NULL) {
        return;
    }
    msgExt = CONTAINER_OF(msg, struct CanMsgHolder, cmsg);
    OsalMemFree(msgExt);
}

static void CanMsgHolderOnLastPut(struct HdfSRef *sref)
{
    struct CanMsgHolder *msgExt = NULL;

    if (sref == NULL) {
        return;
    }
    msgExt = CONTAINER_OF(sref, struct CanMsgHolder, ref);
    CanMsgDoRecycle(&msgExt->cmsg);
}

struct IHdfSRefListener g_canMsgExtListener = {
    .OnFirstAcquire = CanMsgHolderOnFirstGet,
    .OnLastRelease = CanMsgHolderOnLastPut,
};

struct CanMsg *CanMsgObtain(void)
{
    struct CanMsgHolder *msgExt = NULL;

    msgExt = (struct CanMsgHolder *)OsalMemCalloc(sizeof(*msgExt));
    if (msgExt == NULL) {
        return NULL;
    }
    HdfSRefConstruct(&msgExt->ref, &g_canMsgExtListener);
    CanMsgGet(&msgExt->cmsg);
    return &msgExt->cmsg;
}

void CanMsgGet(struct CanMsg *msg)
{
    struct CanMsgHolder *msgExt = NULL;

    if (msg == NULL) {
        return;
    }
    msgExt = CONTAINER_OF(msg, struct CanMsgHolder, cmsg);
    HdfSRefAcquire(&msgExt->ref);
}

void CanMsgPut(struct CanMsg *msg)
{
    struct CanMsgHolder *msgExt = NULL;

    if (msg == NULL) {
        return;
    }
    msgExt = CONTAINER_OF(msg, struct CanMsgHolder, cmsg);
    HdfSRefRelease(&msgExt->ref);
}
