/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can/can_core.h"
#include "can/can_mail.h"
#include "can/can_msg.h"

static int32_t CanCntlrLock(struct CanCntlr *cntlr)
{
    if (cntlr->ops != NULL && cntlr->ops->lock != NULL) {
        return cntlr->ops->lock(cntlr);
    } else {
        return OsalMutexLock(&cntlr->lock);
    }
}

static void CanCntlrUnlock(struct CanCntlr *cntlr)
{
    if (cntlr->ops != NULL && cntlr->ops->unlock != NULL) {
        cntlr->ops->unlock(cntlr);
    } else {
        (void)OsalMutexUnlock(&cntlr->lock);
    }
}

int32_t CanCntlrWriteMsg(struct CanCntlr *cntlr, const struct CanMsg *msg)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->sendMsg == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((ret = CanCntlrLock(cntlr)) != HDF_SUCCESS) {
        return ret;
    }
    ret = cntlr->ops->sendMsg(cntlr, msg);
    CanCntlrUnlock(cntlr);
    return ret;
}

int32_t CanCntlrAddFilter(struct CanCntlr *cntlr, struct CanFilter *filter)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->addFilter == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((ret = CanCntlrLock(cntlr)) != HDF_SUCCESS) {
        return ret;
    }
    ret = cntlr->ops->addFilter(cntlr, filter);
    CanCntlrUnlock(cntlr);
    return ret;
}

int32_t CanCntlrDelFilter(struct CanCntlr *cntlr, struct CanFilter *filter)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->delFilter == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((ret = CanCntlrLock(cntlr)) != HDF_SUCCESS) {
        return ret;
    }
    ret = cntlr->ops->delFilter(cntlr, filter);
    CanCntlrUnlock(cntlr);
    return ret;
}

int32_t CanCntlrSetCfg(struct CanCntlr *cntlr, const struct CanConfig *cfg)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setCfg == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((ret = CanCntlrLock(cntlr)) != HDF_SUCCESS) {
        return ret;
    }
    ret = cntlr->ops->setCfg(cntlr, cfg);
    CanCntlrUnlock(cntlr);
    return ret;
}

int32_t CanCntlrGetCfg(struct CanCntlr *cntlr, struct CanConfig *cfg)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getCfg == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((ret = CanCntlrLock(cntlr)) != HDF_SUCCESS) {
        return ret;
    }
    ret = cntlr->ops->getCfg(cntlr, cfg);
    CanCntlrUnlock(cntlr);
    return ret;
}

int32_t CanCntlrGetState(struct CanCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getState == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((ret = CanCntlrLock(cntlr)) != HDF_SUCCESS) {
        return ret;
    }
    ret = cntlr->ops->getState(cntlr);
    CanCntlrUnlock(cntlr);
    return ret;
}

int32_t CanCntlrAddRxBox(struct CanCntlr *cntlr, struct CanRxBox *rxBox)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (rxBox == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    (void)OsalMutexLock(&cntlr->rboxListLock);
    DListInsertTail(&rxBox->node, &cntlr->rxBoxList);
    (void)OsalMutexUnlock(&cntlr->rboxListLock);
    return HDF_SUCCESS;
}

int32_t CanCntlrDelRxBox(struct CanCntlr *cntlr, struct CanRxBox *rxBox)
{
    struct CanRxBox *tmp = NULL;
    struct CanRxBox *toRmv = NULL;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (rxBox == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    (void)OsalMutexLock(&cntlr->rboxListLock);
    DLIST_FOR_EACH_ENTRY_SAFE(toRmv, tmp, &cntlr->rxBoxList, struct CanRxBox, node) {
        if (toRmv == rxBox) {
            DListRemove(&toRmv->node);
            (void)OsalMutexUnlock(&cntlr->rboxListLock);
            return HDF_SUCCESS;
        }
    }
    (void)OsalMutexUnlock(&cntlr->rboxListLock);
    return HDF_ERR_NOT_SUPPORT;
}

static int32_t CanCntlrMsgDispatch(struct CanCntlr *cntlr, struct CanMsg *msg)
{
    struct CanRxBox *rxBox = NULL;

    (void)OsalMutexLock(&cntlr->rboxListLock);
    DLIST_FOR_EACH_ENTRY(rxBox, &cntlr->rxBoxList, struct CanRxBox, node) {
        (void)CanRxBoxAddMsg(rxBox, msg);
    }
    (void)OsalMutexUnlock(&cntlr->rboxListLock);
    CanMsgPut(msg);

    return HDF_SUCCESS;
}

int32_t CanCntlrOnNewMsg(struct CanCntlr *cntlr, struct CanMsg *msg)
{
    return CanCntlrMsgDispatch(cntlr, msg); // gona call in thread context later ...
}
