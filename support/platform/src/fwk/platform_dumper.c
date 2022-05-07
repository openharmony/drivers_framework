/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "platform_dumper.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_sem.h"
#include "securec.h"

#ifdef __LITEOS__
#define DUMPER_PRINT(fmt, args...) dprintf(fmt, ##args)
#else
#define DUMPER_PRINT(fmt, args...) printk(fmt, ##args)
#endif

static void DumperPrintUint8Info(struct DumperDataMgrNode *data)
{
    uint8_t *val = (uint8_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintUint16Info(struct DumperDataMgrNode *data)
{
    uint16_t *val = (uint16_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintUint32Info(struct DumperDataMgrNode *data)
{
    uint32_t *val = (uint32_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintUint64Info(struct DumperDataMgrNode *data)
{
    uint64_t *val = (uint64_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintInt8Info(struct DumperDataMgrNode *data)
{
    int8_t *val = (int8_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintInt16Info(struct DumperDataMgrNode *data)
{
    int16_t *val = (int16_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintInt32Info(struct DumperDataMgrNode *data)
{
    int32_t *val = (int32_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintInt64Info(struct DumperDataMgrNode *data)
{
    int64_t *val = (int64_t *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintFloatInfo(struct DumperDataMgrNode *data)
{
#ifdef __LITEOS__
    float *val = (float *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
#endif
}

static void DumperPrintDoubleInfo(struct DumperDataMgrNode *data)
{
#ifdef __LITEOS__
    double *val = (double *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
#endif
}

static void DumperPrintCharInfo(struct DumperDataMgrNode *data)
{
    char *val = (char *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, *val);
}

static void DumperPrintStringInfo(struct DumperDataMgrNode *data)
{
    char *val = (char *)(data->data.paddr);
    DUMPER_PRINT(data->printFormat, data->data.name, val);
}

static void DumperPrintRegisterInfo(struct DumperDataMgrNode *data)
{
    unsigned long value;
    volatile uint8_t *regAddr = (volatile uint8_t *)data->data.paddr;
    if (data->data.type == PLATFORM_DUMPER_REGISTERL) {
        value = OSAL_READL(regAddr);
    } else if (data->data.type == PLATFORM_DUMPER_REGISTERW) {
        value = OSAL_READW(regAddr);
    } else if (data->data.type == PLATFORM_DUMPER_REGISTERB) {
        value = OSAL_READB(regAddr);
    } else {
        HDF_LOGE("DumperPrintRegisterInfo illegal type %d", data->data.type);
        return;
    }

    DUMPER_PRINT(data->printFormat, data->data.name, value);
}

struct PlatformDumperDataFormatter {
    enum PlatformDumperDataType type;
    const char *printFormat; // type/name/value
    void (*printFunc)(struct DumperDataMgrNode *data);
};

static struct PlatformDumperDataFormatter g_printInfos[] = {
    {PLATFORM_DUMPER_UINT8,     "uint8           %-32s       %hhu\t", DumperPrintUint8Info   },
    {PLATFORM_DUMPER_UINT16,    "uint16          %-32s       %hu\t",  DumperPrintUint16Info  },
    {PLATFORM_DUMPER_UINT32,    "uint32          %-32s       %u\t",   DumperPrintUint32Info  },
    {PLATFORM_DUMPER_UINT64,    "uint64          %-32s       %llu\t", DumperPrintUint64Info  },
    {PLATFORM_DUMPER_INT8,      "int8            %-32s       %d\t",   DumperPrintInt8Info    },
    {PLATFORM_DUMPER_INT16,     "int16           %-32s       %d\t",   DumperPrintInt16Info   },
    {PLATFORM_DUMPER_INT32,     "int32           %-32s       %d\t",   DumperPrintInt32Info   },
    {PLATFORM_DUMPER_INT64,     "int64           %-32s       %ld\t",  DumperPrintInt64Info   },
    {PLATFORM_DUMPER_CHAR,      "char            %-32s       %c\t",   DumperPrintCharInfo    },
    {PLATFORM_DUMPER_STRING,    "string          %-32s       %s\t",   DumperPrintStringInfo  },
    {PLATFORM_DUMPER_FLOAT,     "float           %-32s       %f\t",   DumperPrintFloatInfo   },
    {PLATFORM_DUMPER_DOUBLE,    "double          %-32s       %lf\t",  DumperPrintDoubleInfo  },
    {PLATFORM_DUMPER_REGISTERL, "registerL       %-32s       %u\t",   DumperPrintRegisterInfo},
    {PLATFORM_DUMPER_REGISTERW, "registerW       %-32s       %hu\t",  DumperPrintRegisterInfo},
    {PLATFORM_DUMPER_REGISTERB, "registerB       %-32s       %hhu\t", DumperPrintRegisterInfo},
};

static int32_t DumperNodeSetPrintInfo(struct DumperDataMgrNode *node)
{
    int i;
    for (i = 0; i < sizeof(g_printInfos) / sizeof(g_printInfos[0]); i++) {
        if (node->data.type == g_printInfos[i].type) {
            node->printFormat = g_printInfos[i].printFormat;
            node->printFunc = g_printInfos[i].printFunc;
            return HDF_SUCCESS;
        }
    }

    HDF_LOGE("DumperNodeSetPrintInfo: node [%s] type [%d] not find", node->data.name, node->data.type);
    return HDF_FAILURE;
}

struct PlatformDumper *PlatformDumperCreate(const char *name)
{
    struct PlatformDumper *dumper = NULL;
    int32_t ret;
    if (name == NULL) {
        HDF_LOGE("PlatformDumperCreate name null");
        return NULL;
    }
    dumper = OsalMemAlloc(sizeof(struct PlatformDumper));
    if (dumper == NULL) {
        HDF_LOGE("PlatformDumperCreate malloc dumper for %s fail", name);
        return NULL;
    }

    dumper->name = name;
    dumper->ops = NULL;
    DListHeadInit(&dumper->dumperDatas);
    ret = OsalSpinInit(&dumper->spin);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("PlatformDumperCreate dumper %s OsalSpinInit fail %d", name, ret);
        OsalMemFree(dumper);
        return NULL;
    }

    HDF_LOGD("PlatformDumperCreate dumper for %s success", name);
    return dumper;
}

static int32_t DumperAddNode(struct PlatformDumper *dumper, struct PlatformDumperData *data)
{
    struct DumperDataMgrNode *pos = NULL;
    struct DumperDataMgrNode *node = NULL;
    if (data == NULL || data->name == NULL || data->paddr == NULL) {
        HDF_LOGE("DumperAddNode: input param illegal");
        return HDF_FAILURE;
    }

    node = OsalMemAlloc(sizeof(struct DumperDataMgrNode));
    if (node == NULL) {
        HDF_LOGE("DumperAddNode: OsalMemAlloc node [%s] fail", data->name);
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(pos, &dumper->dumperDatas, struct DumperDataMgrNode, node) {
        if ((strcmp(pos->data.name, data->name) == 0) && (pos->data.type == data->type)) {
            HDF_LOGE("DumperAddNode: node [%s][%d] existed", data->name, data->type);
            OsalMemFree(node);
            return HDF_FAILURE;
        }
    }

    node->data.name = data->name;
    node->data.type = data->type;
    node->data.paddr = data->paddr;
    if (DumperNodeSetPrintInfo(node) == HDF_FAILURE) {
        HDF_LOGE("DumperAddNode: SetPrintInfo node [%s] fail", data->name);
        OsalMemFree(node);
        return HDF_FAILURE;
    }
    DListInsertTail(&node->node, &dumper->dumperDatas);

    return HDF_SUCCESS;
}

int32_t PlatformDumperAddData(struct PlatformDumper *dumper, struct PlatformDumperData *data)
{
    int32_t ret;
    if (dumper == NULL) {
        HDF_LOGE("PlatformDumperAddData: input param illegal");
        return HDF_FAILURE;
    }
    (void)OsalSpinLock(&dumper->spin);
    ret = DumperAddNode(dumper, data);
    (void)OsalSpinUnlock(&dumper->spin);

    return ret;
}

int32_t PlatformDumperAddDatas(struct PlatformDumper *dumper, struct PlatformDumperData datas[], int size)
{
    int i;
    if (dumper == NULL || datas == NULL) {
        HDF_LOGE("PlatformDumperAddDatas: input param illegal");
        return HDF_FAILURE;
    }
    (void)OsalSpinLock(&dumper->spin);
    for (i = 0; i < size; i++) {
        if (DumperAddNode(dumper, &datas[i]) != HDF_SUCCESS) {
            HDF_LOGE("PlatformDumperAddDatas: [%s] add data [%s] fail", dumper->name, datas[i].name);
            (void)OsalSpinUnlock(&dumper->spin);
            return HDF_FAILURE;
        }
    }
    (void)OsalSpinUnlock(&dumper->spin);

    return HDF_SUCCESS;
}

int32_t PlatformDumperDelData(struct PlatformDumper *dumper, const char *name, enum PlatformDumperDataType type)
{
    struct DumperDataMgrNode *pos = NULL;
    struct DumperDataMgrNode *tmp = NULL;
    if (dumper == NULL || name == NULL) {
        HDF_LOGE("PlatformDumperDelData: input param illegal");
        return HDF_FAILURE;
    }

    (void)OsalSpinLock(&dumper->spin);
    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &dumper->dumperDatas, struct DumperDataMgrNode, node) {
        if ((strcmp(pos->data.name, name) == 0) && (pos->data.type == type)) {
            HDF_LOGD("PlatformDumperDelData: node [%s][%d] find, then del", name, type);
            DListRemove(&pos->node);
            OsalMemFree(pos);
            (void)OsalSpinUnlock(&dumper->spin);
            return HDF_SUCCESS;
        }
    }

    (void)OsalSpinUnlock(&dumper->spin);
    return HDF_SUCCESS;
}

int32_t PlatformDumperClearDatas(struct PlatformDumper *dumper)
{
    struct DumperDataMgrNode *pos = NULL;
    struct DumperDataMgrNode *tmp = NULL;
    if (dumper == NULL) {
        HDF_LOGE("PlatformDumperClearDatas: dumper null");
        return HDF_FAILURE;
    }

    (void)OsalSpinLock(&dumper->spin);
    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &dumper->dumperDatas, struct DumperDataMgrNode, node) {
        DListRemove(&pos->node);
        OsalMemFree(pos);
    }

    (void)OsalSpinUnlock(&dumper->spin);
    return HDF_SUCCESS;
}

void PlatformDumperDestroy(struct PlatformDumper *dumper)
{
    int32_t ret;
    if (dumper == NULL) {
        HDF_LOGE("PlatformDumperDestroy dumper null");
        return;
    }

    PlatformDumperClearDatas(dumper);
    ret = OsalSpinDestroy(&dumper->spin);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("PlatformDumperDestroy dumper %s OsalSpinDestroy fail %d", dumper->name, ret);
    }

    OsalMemFree(dumper);
}

int32_t PlatformDumperSetMethod(struct PlatformDumper *dumper, struct PlatformDumperMethod *ops)
{
    if (dumper == NULL || ops == NULL) {
        HDF_LOGE("PlatformDumperSetMethod: input param null");
        return HDF_FAILURE;
    }
    (void)OsalSpinLock(&dumper->spin);
    dumper->ops = ops;
    (void)OsalSpinUnlock(&dumper->spin);
    return HDF_SUCCESS;
}

static void PlatformGetDumperOpsInfo(struct PlatformDumperMethod *ops)
{
    if (ops == NULL) {
        return;
    }
    if (ops->dumperCfgInfo != NULL) {
        ops->dumperCfgInfo();
    }
    if (ops->dumperStatusInfo != NULL) {
        ops->dumperStatusInfo();
    }
    if (ops->dumperRegisterInfo != NULL) {
        ops->dumperRegisterInfo();
    }
    if (ops->dumperStatisInfo != NULL) {
        ops->dumperStatisInfo();
    }
}

int32_t PlatformDumperDump(struct PlatformDumper *dumper)
{
    struct DumperDataMgrNode *pos = NULL;
    if (dumper == NULL) {
        HDF_LOGE("PlatformDumperDump: dumper null");
        return HDF_FAILURE;
    }

    PlatformGetDumperOpsInfo(dumper->ops);
    DUMPER_PRINT("The dumper %s's data list as follows:\t", dumper->name);
    DUMPER_PRINT("type               name                              value\t");

    (void)OsalSpinLock(&dumper->spin);
    DLIST_FOR_EACH_ENTRY(pos, &dumper->dumperDatas, struct DumperDataMgrNode, node) {
        pos->printFunc(pos);
    }
    (void)OsalSpinUnlock(&dumper->spin);
    return HDF_SUCCESS;
}
