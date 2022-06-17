/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can/can_client.h"
#include "can/can_msg.h"
#include "osal_mem.h"
#include "securec.h"

struct CanClient {
    struct CanCntlr *cntlr;
    struct CanRxBox *rxBox; // receive message box
};

static int32_t CanClientAttach(struct CanClient *client, struct CanCntlr *cntlr)
{
    int32_t ret;

    ret = CanCntlrAddRxBox(cntlr, client->rxBox);
    if (ret == HDF_SUCCESS) {
        client->cntlr = cntlr;
    }
    return ret;
}

static int32_t CanClientDetach(struct CanClient *client)
{
    int32_t ret;

    ret = CanCntlrDelRxBox(client->cntlr, client->rxBox);
    if (ret == HDF_SUCCESS) {
        client->cntlr = NULL;
    }
    return ret;
}

static int32_t CanClientCreate(struct CanCntlr *cntlr, struct CanClient **client)
{
    int32_t ret;
    struct CanClient *new = NULL;

    new = (struct CanClient *)OsalMemCalloc(sizeof(*new));
    if (new == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    new->rxBox = CanRxBoxCreate();
    if (new->rxBox == NULL) {
        OsalMemFree(new);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = CanClientAttach(new, cntlr);
    if (ret != HDF_SUCCESS) {
        CanRxBoxDestroy(new->rxBox);
        OsalMemFree(new);
        return ret;
    }

    *client = new;
    return HDF_SUCCESS;
}

int32_t CanClientCreateByNumber(int32_t busNum, struct CanClient **client)
{
    int32_t ret;
    struct CanCntlr *cntlr = NULL;

    cntlr = CanCntlrGetByNumber(busNum);
    if (cntlr == NULL) {
        return HDF_PLT_ERR_DEV_GET;
    }

    ret = CanClientCreate(cntlr, client);
    if (ret != HDF_SUCCESS) {
        CanCntlrPut(cntlr);
        return ret;
    }
    return HDF_SUCCESS;
}

void CanClientDestroy(struct CanClient *client)
{
    struct CanCntlr *cntlr = NULL;

    if (client != NULL) {
        cntlr = client->cntlr;
        CanClientDetach(client);
        CanCntlrPut(cntlr);
        CanRxBoxDestroy(client->rxBox);
        OsalMemFree(client);
    }
}

int32_t CanClientWriteMsg(struct CanClient *client, const struct CanMsg *msg)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return CanCntlrWriteMsg(client->cntlr, msg);
}

int32_t CanClientReadMsg(struct CanClient *client, struct CanMsg *msg, uint32_t tms)
{
    int32_t ret;
    struct CanMsg *cmsg = NULL;

    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = CanRxBoxGetMsg(client->rxBox, &cmsg, tms);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = memcpy_s(msg, sizeof(*msg), cmsg, sizeof(*cmsg));
    CanMsgPut(cmsg); // decrease ref cnt after mem cpoy
    return (ret == EOK) ? HDF_SUCCESS : HDF_ERR_IO;
}

int32_t CanClientAddFilter(struct CanClient *client, const struct CanFilter *filter)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return CanRxBoxAddFilter(client->rxBox, filter);
}

int32_t CanClientDelFilter(struct CanClient *client, const struct CanFilter *filter)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return CanRxBoxDelFilter(client->rxBox, filter);
}

int32_t CanClientSetCfg(struct CanClient *client, const struct CanConfig *cfg)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return CanCntlrSetCfg(client->cntlr, cfg);
}

int32_t CanClientGetCfg(struct CanClient *client, struct CanConfig *cfg)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return CanCntlrGetCfg(client->cntlr, cfg);
}

int32_t CanClientGetState(struct CanClient *client)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return CanCntlrGetState(client->cntlr);
}
