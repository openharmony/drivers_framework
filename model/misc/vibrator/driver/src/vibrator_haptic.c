/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "securec.h"
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_mem.h"
#include "vibrator_driver.h"
#include "vibrator_haptic.h"

#define HDF_LOG_TAG    vibrator_haptic_c
#define VIBRATOR_HAPTIC_STACK_SIZE    0x2000
#define VIBRATOR_HAPTIC_SEQ_MAX       1024
#define VIBRATOR_HAPTIC_SEQ_SIZE       4
#define VIBRATOR_HAPTIC_SEQ_NAME_MAX    48
#define VIBRATOR_HAPTIC_SEQ_SIZE_MAX    (4 * VIBRATOR_HAPTIC_SEQ_MAX)

struct VibratorHapticData *g_vibratorHapticData = NULL;

static struct VibratorHapticData *GetHapticData(void)
{
    return g_vibratorHapticData;
}

static struct VibratorEffectNode *MallocEffectNode(int32_t seqSize)
{
    uint32_t *seq = NULL;
    struct VibratorEffectNode *node = NULL;

    if (seqSize <= 0 || seqSize > VIBRATOR_HAPTIC_SEQ_SIZE_MAX) {
        HDF_LOGE("%s: malloc ", __func__);
        return NULL;
    }

    seq = (uint32_t *)OsalMemCalloc(seqSize);
    if (seq == NULL) {
        HDF_LOGE("%s: malloc seq fail", __func__);
        return NULL;
    }

    node = (struct VibratorEffectNode *)OsalMemCalloc(sizeof(*node));
    if (node == NULL) {
        HDF_LOGE("%s: malloc seq fail", __func__);
        OsalMemFree(seq);
        return NULL;
    }
    node->seq = seq;

    return node;
}

static int32_t ParserHapticEffect(struct DeviceResourceIface *parser, const struct DeviceResourceNode *hapticNode)
{
    int32_t ret;
    int32_t count;
    struct VibratorEffectNode *effectNode = NULL;
    const struct DeviceResourceAttr *hapticAttr = NULL;
    struct VibratorHapticData *hapticData = GetHapticData();

    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_FAILURE);

    (void)OsalMutexLock(&hapticData->mutex);
    DEV_RES_NODE_FOR_EACH_ATTR(hapticNode, hapticAttr) {
        if ((hapticAttr == NULL) || (hapticAttr->name == NULL)) {
            break;
        }

        count = parser->GetElemNum(hapticNode, hapticAttr->name);
        // Minimum of two elements, including the type and sequence.
        if (count <= 1 || count > VIBRATOR_HAPTIC_SEQ_MAX) {
            HDF_LOGD("%s: haptic [%s] parser seq count fail", __func__, hapticAttr->name);
            continue;
        }

        effectNode = MallocEffectNode(count * VIBRATOR_HAPTIC_SEQ_SIZE);
        if (effectNode == NULL) {
            HDF_LOGD("%s: malloc effect effectNode fail", __func__);
            continue;
        }
        effectNode->effect = hapticAttr->name;
        ret = parser->GetUint32Array(hapticNode, hapticAttr->name, effectNode->seq, count, 0);
        CHECK_VIBRATOR_PARSER_RESULT_RETURN_VALUE(ret, hapticAttr->name);
        effectNode->num = count;
        DListInsertTail(&effectNode->node, &hapticData->effectSeqHead);
    }
    (void)OsalMutexUnlock(&hapticData->mutex);

    if (DListIsEmpty(&hapticData->effectSeqHead)) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t ParserVibratorHapticConfig(const struct DeviceResourceNode *node)
{
    bool supportPresetFlag = false;
    struct DeviceResourceIface *parser = NULL;
    const struct DeviceResourceNode *vibratorAttrNode = NULL;
    const struct DeviceResourceNode *vibratorHapticNode = NULL;
    struct VibratorHapticData *hapticData = GetHapticData();

    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_FAILURE);
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(node, HDF_ERR_INVALID_PARAM);

    parser = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(parser, HDF_ERR_INVALID_PARAM);
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(parser->GetChildNode, HDF_ERR_INVALID_PARAM);

    // get haptic type
    vibratorAttrNode = parser->GetChildNode(node, "vibratorAttr");
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(vibratorAttrNode, HDF_ERR_INVALID_PARAM);
    supportPresetFlag = parser->GetBool(vibratorAttrNode, "supportPreset");
    if (!supportPresetFlag) {
        HDF_LOGD("%s: vibrator not support effect", __func__);
        return HDF_SUCCESS;
    }

    (void)OsalMutexLock(&hapticData->mutex);
    hapticData->supportPreset = supportPresetFlag;
    (void)OsalMutexUnlock(&hapticData->mutex);

    // malloc haptic resource
    vibratorHapticNode = parser->GetChildNode(node, "vibratorHapticConfig");
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(vibratorHapticNode, HDF_ERR_INVALID_PARAM);
    if (ParserHapticEffect(parser, vibratorHapticNode) != HDF_SUCCESS) {
        HDF_LOGE("%s: vibrator get effect fail", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DelayTimeForEffect(struct OsalSem *sem, uint32_t time)
{
    int32_t ret;
    if (time == 0) {
        return HDF_SUCCESS;
    }

    ret = OsalSemWait(sem, time);
    if (ret == 0) {
        HDF_LOGD("%s: haptic need break", __func__);
        return -1;
    }

    return HDF_SUCCESS;
}

static int32_t HapticThreadEntry(void *para)
{
    int32_t count;
    int32_t index = 1;
    uint32_t delaytime;
    uint32_t effect;
    struct VibratorHapticData *hapticData = NULL;

    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(para, HDF_FAILURE);
    hapticData = (struct VibratorHapticData *)para;
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData->currentEffectSeq, HDF_FAILURE);

    count = hapticData->seqCount;
    if (count <= 1 || count > VIBRATOR_HAPTIC_SEQ_MAX) {
        HDF_LOGE("%s: count para invalid", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    while (!hapticData->threadExitFlag) {
        if (index + 1 >= count) {
            break;
        }

        delaytime = hapticData->currentEffectSeq[index];
        if (DelayTimeForEffect(&hapticData->hapticSem, delaytime) < 0) {
            continue;
        }

        index++;
        effect = hapticData->currentEffectSeq[index++];
        if (hapticData->effectType == VIBRATOR_TYPE_EFFECT) {
            SetEffectVibrator(effect);
        } else if (hapticData->effectType == VIBRATOR_TYPE_TIME) {
            StartTimeVibrator(effect);
        }
    }

    (void)OsalMutexLock(&hapticData->mutex);
    hapticData->threadExitFlag = true;
    (void)OsalMutexUnlock(&hapticData->mutex);

    return HDF_SUCCESS;
}

int32_t InitVibratorHaptic(struct HdfDeviceObject *device)
{
    struct VibratorHapticData *hapticData = NULL;
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(device, HDF_FAILURE);

    hapticData = (struct VibratorHapticData *)OsalMemCalloc(sizeof(*hapticData));
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_ERR_MALLOC_FAIL);
    g_vibratorHapticData = hapticData;
    hapticData->threadExitFlag = true;
    hapticData->supportPreset = false;

    if (OsalMutexInit(&hapticData->mutex) != HDF_SUCCESS) {
        HDF_LOGE("%s: fail to create thread lock", __func__);
        goto EXIT;
    }

    if (OsalSemInit(&hapticData->hapticSem, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: fail to create thread lock", __func__);
        goto EXIT;
    }

    // create thread
    if (OsalThreadCreate(&hapticData->hapticThread, HapticThreadEntry, (void *)hapticData) != HDF_SUCCESS) {
        HDF_LOGE("%s: create haptic thread fail!", __func__);
        goto EXIT;
    }
    DListHeadInit(&hapticData->effectSeqHead);

    // get haptic hcs
    if (ParserVibratorHapticConfig(device->property) != HDF_SUCCESS) {
        HDF_LOGE("%s: parser haptic config fail!", __func__);
        goto EXIT;
    }

    return HDF_SUCCESS;
EXIT:
    OsalMemFree(hapticData);
    return HDF_FAILURE;
}

static int32_t GetHapticSeqByEffect(struct VibratorEffectCfg *effectCfg)
{
    struct VibratorHapticData *hapticData = NULL;
    struct VibratorEffectNode *pos = NULL;
    struct VibratorEffectNode *tmp = NULL;

    hapticData = GetHapticData();
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_FAILURE);

    if (effectCfg->cfgMode == VIBRATOR_MODE_ONCE) {
        (void)OsalMutexLock(&hapticData->mutex);
        hapticData->duration = effectCfg->duration;
        hapticData->effectType = VIBRATOR_TYPE_TIME;
        (void)OsalMutexUnlock(&hapticData->mutex);
    } else if (effectCfg->cfgMode == VIBRATOR_MODE_PRESET) {
        if (effectCfg->effect == NULL) {
            HDF_LOGE("%s: effect para invalid!", __func__);
            return HDF_FAILURE;
        }

        hapticData->seqCount = 0;
        hapticData->currentEffectSeq = NULL;
        DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &hapticData->effectSeqHead, struct VibratorEffectNode, node) {
            if (strcmp(effectCfg->effect, pos->effect) == 0 && pos->seq != NULL) {
                (void)OsalMutexLock(&hapticData->mutex);
                hapticData->effectType = pos->seq[0];
                hapticData->seqCount = pos->num - 1;
                hapticData->currentEffectSeq = &(pos->seq[1]);
                (void)OsalMutexUnlock(&hapticData->mutex);
                break;
            }
        }
        if (hapticData->seqCount <= 0) {
            HDF_LOGE("%s: not find effect type!", __func__);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t StartHaptic(struct VibratorEffectCfg *effectCfg)
{
    int32_t ret;
    struct OsalThreadParam threadCfg;
    struct VibratorHapticData *hapticData = GetHapticData();
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(effectCfg, HDF_FAILURE);
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_FAILURE);

    if (!hapticData->threadExitFlag) {
        OsalSemPost(&hapticData->hapticSem);
        return HDF_SUCCESS;
    }

    ret = GetHapticSeqByEffect(effectCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get haptic seq fail!", __func__);
        return ret;
    }

    (void)OsalMutexLock(&hapticData->mutex);
    hapticData->threadExitFlag = false;
    (void)OsalMutexUnlock(&hapticData->mutex);

    threadCfg.name = "vibrator_haptic";
    threadCfg.priority = OSAL_THREAD_PRI_DEFAULT;
    threadCfg.stackSize = VIBRATOR_HAPTIC_STACK_SIZE;
    ret = OsalThreadStart(&hapticData->hapticThread, &threadCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: start haptic thread fail!", __func__);
        return ret;
    }

    return HDF_SUCCESS;
}

int32_t StopHaptic()
{
    struct VibratorHapticData *hapticData = GetHapticData();
    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_FAILURE);

    if (hapticData->threadExitFlag) {
        return HDF_SUCCESS;
    }

    (void)OsalMutexLock(&hapticData->mutex);
    hapticData->threadExitFlag = true;
    (void)OsalMutexUnlock(&hapticData->mutex);
    OsalSemPost(&hapticData->hapticSem);
    StopVibrator();

    return HDF_SUCCESS;
}

static void ReleaseHapticConfig()
{
    struct VibratorHapticData *hapticData = GetHapticData();
    struct VibratorEffectNode *pos = NULL;
    struct VibratorEffectNode *tmp = NULL;

    if (hapticData == NULL) {
        return;
    }

    (void)OsalMutexLock(&hapticData->mutex);
    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &hapticData->effectSeqHead, struct VibratorEffectNode, node) {
        if (pos->seq != NULL) {
            OsalMemFree(pos->seq);
            pos->seq = NULL;
        }
        pos->effect = NULL;
        DListRemove(&pos->node);
        OsalMemFree(pos);
    }
    (void)OsalMutexUnlock(&hapticData->mutex);
}

int32_t DestroyHaptic()
{
    struct VibratorHapticData *hapticData = GetHapticData();

    CHECK_VIBRATOR_NULL_PTR_RETURN_VALUE(hapticData, HDF_FAILURE);
    if (hapticData->supportPreset) {
        ReleaseHapticConfig();
    }

    (void)OsalMutexDestroy(&hapticData->mutex);
    (void)OsalSemDestroy(&hapticData->hapticSem);
    (void)OsalThreadDestroy(&hapticData->hapticThread);

    return HDF_SUCCESS;
}
