/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can_test.h"
#include "can_if.h"
#include "device_resource_if.h"
#include "hdf_device_desc.h"
#include "hdf_io_service_if.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_thread.h"
#include "osal_time.h"
#include "platform_assert.h"
#include "securec.h"

#define HDF_LOG_TAG         can_test
#define CAN_TEST_STACK_SIZE (1024 * 10)

#define CAN_TEST_ID_A 0x15A
#define CAN_TEST_ID_B 0x2A5
#define CAN_TEST_ID_C 0x555
#define CAN_MASK_FULL 0x1FFFFFFF
#define CAN_TEST_DATA 0xAB

#define CAN_TEST_TIMEOUT_20 20
#define CAN_TEST_TIMEOUT_10 10

static struct HdfDeviceObject hdfDev = {
    .service = NULL,
    .property = NULL,
    .priv = NULL,
};

struct HdfDriverEntry *CanVirtualGetEntry(void);
struct HdfDriverEntry *g_driverEntry = NULL;
static uint16_t g_busNum;
static DevHandle g_handle;
static struct CanMsg g_msgA;
static struct CanMsg g_msgB;
static struct CanMsg g_msgC;

static struct CanFilter g_filterA = {
    .rtr = 0,
    .ide = 0,
    .id = CAN_TEST_ID_A,
    .rtrMask = 1,
    .ideMask = 1,
    .idMask = CAN_MASK_FULL,
};

static struct CanFilter g_filterB = {
    .rtr = 0,
    .ide = 0,
    .id = CAN_TEST_ID_B,
    .rtrMask = 1,
    .ideMask = 1,
    .idMask = CAN_MASK_FULL,
};

static void CanMsgInitByParms(struct CanMsg *msg, uint32_t id, uint32_t ide, uint32_t rtr, uint8_t data)
{
    msg->ide = ide;
    msg->id = id;
    msg->rtr = rtr;
    if (memset_s(msg->data, sizeof(msg->data), data, sizeof(msg->data)) != EOK) {
        HDF_LOGW("CanMsgInitByParms: init data failed");
    }
    msg->dlc = sizeof(msg->data);
    msg->error = 0;
}

static int32_t CanTestGetConfig(struct CanTestConfig *config)
{
    int32_t ret;
    struct HdfSBuf *reply = NULL;
    struct HdfIoService *service = NULL;
    const void *buf = NULL;
    uint32_t len;

    HDF_LOGD("CanTestGetConfig: enter!");
    service = HdfIoServiceBind("CAN_TEST");
    if (service == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    reply = HdfSbufObtain(sizeof(*config) + sizeof(uint64_t));
    if (reply == NULL) {
        HDF_LOGE("CanTestGetConfig: failed to obtain reply!");
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = service->dispatcher->Dispatch(&service->object, 0, NULL, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("CanTestGetConfig: remote dispatch fail:%d", ret);
    }

    if (!HdfSbufReadBuffer(reply, &buf, &len)) {
        HDF_LOGE("CanTestGetConfig: read buf fail!");
        HdfSbufRecycle(reply);
        return HDF_ERR_IO;
    }

    if (len != sizeof(*config)) {
        HDF_LOGE("CanTestGetConfig: config size:%u, but read size:%u!", sizeof(*config), len);
        HdfSbufRecycle(reply);
        return HDF_ERR_IO;
    }

    if (memcpy_s(config, sizeof(*config), buf, sizeof(*config)) != EOK) {
        HDF_LOGE("CanTestGetConfig: memcpy buf fail!");
        HdfSbufRecycle(reply);
        return HDF_ERR_IO;
    }

    HDF_LOGE("CanTestGetConfig: test on bus:0x%x, bitRate:%u, workMode:0x%x", config->busNum, config->bitRate,
        config->workMode);
    HdfSbufRecycle(reply);
    HDF_LOGD("CanTestGetConfig: exit!");
    HdfIoServiceRecycle(service);
    return HDF_SUCCESS;
}

static int32_t CanTestSetUpByConfig(struct CanTestConfig *config)
{
    struct CanConfig cfg;
    g_busNum = config->busNum;
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusOpen(g_busNum, &g_handle));
    CHECK_FALSE_RETURN(g_handle == NULL);
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusGetCfg(g_handle, &cfg));
    cfg.speed = config->bitRate;
    cfg.mode = config->workMode;
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSetCfg(g_handle, &cfg));
    CanMsgInitByParms(&g_msgA, CAN_TEST_ID_A, 0, 0, CAN_TEST_DATA);
    CanMsgInitByParms(&g_msgB, CAN_TEST_ID_B, 0, 0, CAN_TEST_DATA);
    CanMsgInitByParms(&g_msgC, CAN_TEST_ID_C, 0, 0, CAN_TEST_DATA);
    return HDF_SUCCESS;
}

int32_t CanTestSetUpEveryCase(void)
{
    struct CanTestConfig config;

    if (CanTestGetConfig(&config) != HDF_SUCCESS) {
        HDF_LOGW("CanTestSetUpEveryCase: get config failed, using default config...");
        config.bitRate = CAN_TEST_BIT_RATE;
        config.workMode = CAN_TEST_WORK_MODE;
        config.busNum = CAN_TEST_BUS_NUM;
        g_driverEntry = CanVirtualGetEntry();
        CHECK_FALSE_RETURN(g_driverEntry == NULL);
        LONGS_EQUAL_RETURN(HDF_SUCCESS, g_driverEntry->Init(&hdfDev));
    }
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanTestSetUpByConfig(&config));
    return HDF_SUCCESS;
}

int32_t CanTestTearDownEveryCase(void)
{
    CanBusClose(g_handle);
    if (g_driverEntry != NULL) {
        g_driverEntry->Release(&hdfDev);
        g_driverEntry = NULL;
    }
    return HDF_SUCCESS;
}

static bool CanMsgEquals(const struct CanMsg *msgA, const struct CanMsg *msgB)
{
    int i;
    if (msgA->ide != msgB->ide) {
        return false;
    }
    if (msgA->id != msgB->id) {
        return false;
    }
    if (msgA->rtr != msgB->rtr) {
        return false;
    }
    if (msgA->dlc != msgB->dlc) {
        return false;
    }
    for (i = 0; i < msgA->dlc; i++) {
        if (msgA->data[i] != msgB->data[i]) {
            return false;
        }
    }
    return true;
}

static bool CanBusCanNotReadMsg(DevHandle handle, struct CanMsg *msg)
{
    struct CanMsg msgGot;
    LONGS_EQUAL_RETURN(HDF_ERR_TIMEOUT, CanBusReadMsg(handle, &msgGot, CAN_TEST_TIMEOUT_10));
    return true;
}

static bool CanBusCanReadMsg(DevHandle handle, struct CanMsg *msg)
{
    struct CanMsg msgGot;
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusReadMsg(handle, &msgGot, CAN_TEST_TIMEOUT_10));
    CHECK_TRUE_RETURN(CanMsgEquals(msg, &msgGot));
    return true;
}

static bool CanBusCanSendAndReadMsg(DevHandle handle, struct CanMsg *msg)
{
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(handle, msg));
    return CanBusCanReadMsg(handle, msg);
}

static int32_t CanTestSendAndRead(void)
{
    CHECK_TRUE_RETURN(CanBusCanSendAndReadMsg(g_handle, &g_msgA));
    return HDF_SUCCESS;
}

static int32_t CanTestNoBlockRead(void)
{
    struct CanMsg msg;
    CHECK_FALSE_RETURN(HDF_ERR_TIMEOUT == CanBusReadMsg(g_handle, &msg, 0));
    return HDF_SUCCESS;
}

static int32_t CanTestBlockRead(void)
{
    struct CanMsg msg;
    LONGS_EQUAL_RETURN(HDF_ERR_TIMEOUT, CanBusReadMsg(g_handle, &msg, CAN_TEST_TIMEOUT_10));
    return HDF_SUCCESS;
}

static int32_t CanTestAddAndDelFilter(void)
{
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusAddFilter(g_handle, &g_filterA));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgA));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgB));
    CHECK_TRUE_RETURN(CanBusCanReadMsg(g_handle, &g_msgA));
    CHECK_TRUE_RETURN(CanBusCanNotReadMsg(g_handle, &g_msgB)); // filter out ...

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusDelFilter(g_handle, &g_filterA));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgB));
    CHECK_TRUE_RETURN(CanBusCanReadMsg(g_handle, &g_msgB)); // filter out ...
    return HDF_SUCCESS;
}

static int32_t CanTestAddMultiFilter(void)
{
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusAddFilter(g_handle, &g_filterA));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusAddFilter(g_handle, &g_filterB));

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgA));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgB));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgC));

    CHECK_TRUE_RETURN(CanBusCanReadMsg(g_handle, &g_msgA));
    CHECK_TRUE_RETURN(CanBusCanReadMsg(g_handle, &g_msgB));
    CHECK_TRUE_RETURN(CanBusCanNotReadMsg(g_handle, &g_msgC));

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusDelFilter(g_handle, &g_filterA));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusDelFilter(g_handle, &g_filterB));

    CHECK_TRUE_RETURN(CanBusCanSendAndReadMsg(g_handle, &g_msgC));
    return HDF_SUCCESS;
}

static int32_t CanTestGetBusState(void)
{
    int32_t ret;
    ret = CanBusGetState(g_handle);
    if (ret == HDF_ERR_NOT_SUPPORT) {
        return HDF_SUCCESS;
    }
    CHECK_TRUE_RETURN(ret >= CAN_BUS_RESET && ret < CAN_BUS_INVALID);
    return HDF_SUCCESS;
}

static struct OsalThread *CanTestStartTestThread(OsalThreadEntry entry, DevHandle handle)
{
    int32_t ret;
    struct OsalThreadParam threadCfg;
    struct OsalThread *thread = (struct OsalThread *)OsalMemCalloc(sizeof(*thread));

    ret = OsalThreadCreate(thread, (OsalThreadEntry)entry, (void *)handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("create test thread fail:%d", ret);
        return NULL;
    }

    threadCfg.name = (char *)"CanTestPoller";
    threadCfg.priority = OSAL_THREAD_PRI_DEFAULT;
    threadCfg.stackSize = CAN_TEST_STACK_SIZE;

    ret = OsalThreadStart(thread, &threadCfg);
    if (ret != HDF_SUCCESS) {
        (void)OsalThreadDestroy(thread);
        HDF_LOGE("start test thread2 fail:%d", ret);
        return NULL;
    }

    return thread;
}

static void CanTestStopTestThread(struct OsalThread *thread)
{
    if (thread == NULL) {
        return;
    }
    (void)OsalThreadDestroy(thread);
    OsalMemFree(thread);
}

static int CanTestReaderFunc(void *param)
{
    struct CanMsg msg;
    DevHandle handle = (DevHandle)param;

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusReadMsg(handle, &msg, CAN_TEST_TIMEOUT_10));
    return HDF_SUCCESS;
}

static int CanTestSenderFunc(void *param)
{
    DevHandle handle = (DevHandle)param;

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(handle, &g_msgA));
    return HDF_SUCCESS;
}

static int32_t CanTestMultiThreadReadSameHandle(void)
{
    struct CanMsg msgGot;
    struct OsalThread *thread = NULL;
    thread = CanTestStartTestThread(CanTestReaderFunc, g_handle);
    CHECK_FALSE_RETURN(thread == NULL);
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgA));
    OsalMSleep(CAN_TEST_TIMEOUT_20);
    CHECK_FALSE_RETURN(CanBusReadMsg(g_handle, &msgGot, 0) == HDF_SUCCESS);
    CanTestStopTestThread(thread);
    return HDF_SUCCESS;
}

static int32_t CanTestMultiThreadReadMultiHandle(void)
{
    struct CanMsg msgGot;
    struct OsalThread *thread = NULL;
    DevHandle handle = NULL;

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusOpen(g_busNum, &handle));
    thread = CanTestStartTestThread(CanTestReaderFunc, handle);
    CHECK_FALSE_RETURN(thread == NULL);
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusSendMsg(g_handle, &g_msgA));
    OsalMSleep(CAN_TEST_TIMEOUT_20);
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusReadMsg(g_handle, &msgGot, 0));
    CanTestStopTestThread(thread);
    CanBusClose(handle);
    return HDF_SUCCESS;
}

static int32_t CanTestMultiThreadSendSameHandle(void)
{
    struct CanMsg msgGot;
    struct OsalThread *thread = NULL;

    thread = CanTestStartTestThread(CanTestSenderFunc, g_handle);
    CHECK_FALSE_RETURN(thread == NULL);
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusReadMsg(g_handle, &msgGot, CAN_TEST_TIMEOUT_10));
    CanTestStopTestThread(thread);
    return HDF_SUCCESS;
}

static int32_t CanTestMultiThreadSendMultiHandle(void)
{
    struct CanMsg msgGot;
    struct OsalThread *thread = NULL;
    DevHandle handle = NULL;

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusOpen(g_busNum, &handle));
    thread = CanTestStartTestThread(CanTestSenderFunc, g_handle);
    CHECK_FALSE_RETURN(thread == NULL);
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusReadMsg(handle, &msgGot, CAN_TEST_TIMEOUT_10));
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanBusReadMsg(g_handle, &msgGot, CAN_TEST_TIMEOUT_10));
    CanTestStopTestThread(thread);
    CanBusClose(handle);
    return HDF_SUCCESS;
}

static int32_t CanTestReliability(void)
{
    int32_t ret;
    struct CanMsg msg;
    struct CanFilter filter;
    struct CanConfig cfg;
    DevHandle handle = g_handle;

    /* invalid device handle */
    ret = CanBusSendMsg(NULL, &msg);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusReadMsg(NULL, &msg, 0);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusReadMsg(NULL, &msg, 1);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusSetCfg(NULL, &cfg);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusGetCfg(NULL, &cfg);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusAddFilter(NULL, &filter);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusDelFilter(NULL, &filter);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusGetState(NULL);
    CHECK_LT_RETURN(ret, 0, HDF_FAILURE);

    /* invalid parmas */
    ret = CanBusSendMsg(handle, NULL);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusReadMsg(handle, NULL, 0);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusReadMsg(handle, NULL, 1);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusSetCfg(handle, NULL);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusGetCfg(handle, NULL);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusAddFilter(handle, &filter);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);
    ret = CanBusDelFilter(handle, &filter);
    CHECK_NE_RETURN(ret, HDF_SUCCESS, HDF_FAILURE);

    return HDF_SUCCESS;
}

struct CanTestEntry {
    int cmd;
    int32_t (*func)(void);
    const char *name;
};

static struct CanTestEntry g_testEntry[] = {
    { CAN_TEST_SEND_AND_READ, CanTestSendAndRead, "should_send_and_read_msg_success" },
    { CAN_TEST_NO_BLOCK_READ, CanTestNoBlockRead, "should_return_immediately_when_read_no_block" },
    { CAN_TEST_BLOCK_READ, CanTestBlockRead, "should_return_timeout_when_read_block" },
    { CAN_TEST_ADD_DEL_FILTER, CanTestAddAndDelFilter, "should_add_and_del_filter" },
    { CAN_TEST_ADD_MULTI_FILTER, CanTestAddMultiFilter, "should_add_multi_filter" },
    { CAN_TEST_GET_BUS_STATE, CanTestGetBusState, "should_get_bus_state_unless_not_supported" },
    { CAN_TEST_MULTI_THREAD_READ_SAME_HANDLE, CanTestMultiThreadReadSameHandle,
        "should_read_success_in_another_thread_by_the_same_handle" },
    { CAN_TEST_MULTI_THREAD_READ_MULTI_HANDLE, CanTestMultiThreadReadMultiHandle,
        "should_read_success_in_another_thread_by_another_handle" },
    { CAN_TEST_MULTI_THREAD_SEND_SAME_HANDLE, CanTestMultiThreadSendSameHandle,
        "should_send_success_in_another_thread_by_the_same_handle" },
    { CAN_TEST_MULTI_THREAD_SEND_MULTI_HANDLE, CanTestMultiThreadSendMultiHandle,
        "should_send_success_in_another_thread_by_another_handle" },
    { CAN_TEST_RELIABILITY, CanTestReliability, "CanTestReliability" },
};

int32_t CanTestExecute(int cmd)
{
    int32_t ret;
    uint32_t i;
    struct CanTestEntry *entry = NULL;

    if (cmd >= CAN_TEST_CMD_MAX) {
        HDF_LOGE("CanTestExecute: invalid cmd:%d", cmd);
        return HDF_ERR_NOT_SUPPORT;
    }

    for (i = 0; i < sizeof(g_testEntry) / sizeof(g_testEntry[0]); i++) {
        if (g_testEntry[i].cmd != cmd || g_testEntry[i].func == NULL) {
            continue;
        }
        entry = &g_testEntry[i];
        break;
    }

    if (entry == NULL) {
        HDF_LOGE("%s: no entry matched, cmd = %d", __func__, cmd);
        return HDF_ERR_NOT_SUPPORT;
    }

    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanTestSetUpEveryCase());
    ret = entry->func();
    LONGS_EQUAL_RETURN(HDF_SUCCESS, CanTestTearDownEveryCase());

    HDF_LOGE("[CanTestExecute][======cmd:%d====ret:%d======]", cmd, ret);
    return ret;
}
