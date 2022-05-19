/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ppg_cs1262.h"
#include <securec.h>
#include "sensor_ppg_driver.h"
#include "sensor_device_if.h"
#include "osal_mem.h"
#include "osal_time.h"

#define HDF_LOG_TAG hdf_sensor_cs1262

static struct Cs1262DrvData *g_cs1262DrvData = NULL;

static struct Cs1262DrvData *GetDrvData(void)
{
    return g_cs1262DrvData;
}

static int32_t ResetChip()
{
    uint8_t resetCmd[] = { CS1262_CHIP_REST_CMD, CS1262_SPI_DUMMY_DATA, CS1262_SPI_DUMMY_DATA };

    if (Cs1262WriteData(resetCmd, HDF_ARRAY_SIZE(resetCmd)) != HDF_SUCCESS) {
        HDF_LOGE("%s: Cs1262WriteData fail", __func__);
        return HDF_FAILURE;
    }

    // delay 1 ms
    OsalMDelay(1);
    return HDF_SUCCESS;
}

static int32_t ResetModule(uint16_t moduleOffset)
{
    uint8_t ret;

    ret = Cs1262WriteRegbit(CS1262_RSTCON_REG, moduleOffset, CS1262_REG_BIT_SET);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: CS1262_RST module fail CS1262_REG_BIT_SET", __func__);
        return HDF_FAILURE;
    }

    ret = Cs1262WriteRegbit(CS1262_RSTCON_REG, moduleOffset, CS1262_REG_BIT_RESET);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: CS1262_RST module fail CS1262_REG_BIT_RESET", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t RegLock(Cs1262LockStatus lockStatus)
{
    int32_t ret;
    uint16_t regRead = 0;
    Cs1262RegGroup lockGroup[] = {
        {.regAddr = CS1262_WRPROT_REG, .regVal = CS1262_LOCK, 1},
        {.regAddr = CS1262_WRPROT_REG, .regVal = CS1262_UN_LOCK1, 1},
        {.regAddr = CS1262_WRPROT_REG, .regVal = CS1262_UN_LOCK2, 1},
        {.regAddr = CS1262_WRPROT_REG, .regVal = CS1262_UN_LOCK3, 1}
    };
    // lock write 1 byte 'CS1262_LOCK'; unlock need write 3 bytes
    uint16_t lockGroupLen = (lockStatus == CS1262_REG_LOCK) ? 1 : 3;

    ret = Cs1262WriteGroup(&lockGroup[lockStatus], lockGroupLen);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Cs1262WriteGroup fail", __func__);
        return HDF_FAILURE;
    }

    ret = Cs1262ReadRegs(CS1262_SYS_STATE_REG, &regRead, 1);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Cs1262ReadRegs CS1262_SYS_STATE_REG fail", __func__);
        return HDF_FAILURE;
    }

    if ((regRead & LOCK_REG_OFFSET) == lockStatus) {
        return HDF_SUCCESS;
    }

    HDF_LOGE("%s: Cs1262 Lock fail status = %d", __func__, lockStatus);
    return HDF_FAILURE;
}

static int32_t ReadFifo(uint8_t *outBuf, uint16_t outBufMaxLen, uint16_t *outLen)
{
    uint8_t ret;
    uint16_t fifoState = 0;
    uint16_t fifoNum;

    ret = Cs1262ReadRegs(CS1262_FIFO_STATE_REG, &fifoState, 1);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read reg %d fail", __func__, CS1262_FIFO_STATE_REG);
        return HDF_FAILURE;
    }

    fifoNum = fifoState & FIFO_NUM_OFFSET;
    // empty
    if ((fifoNum == 0) || (fifoState & FIFO_EMPTY_OFFSET)) {
        HDF_LOGI("%s: data FIFO is empty, no need read.", __func__);
        return HDF_SUCCESS;
    }

    // full
    if (fifoState & FIFO_FULL_OFFSET) {
        fifoNum = CS1262_MAX_FIFO_READ_NUM;
    }

    // overflow
    if ((fifoNum * sizeof(Cs1262FifoVal)) > outBufMaxLen) {
        fifoNum = (outBufMaxLen / sizeof(Cs1262FifoVal));
    }

    (void)memset_s(outBuf, outBufMaxLen, 0, (fifoNum * sizeof(Cs1262FifoVal)));
    ret = Cs1262ReadFifoReg((Cs1262FifoVal *)outBuf, fifoNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: ReadFifoReg fail", __func__);
        return HDF_FAILURE;
    }

    *outLen = fifoNum * sizeof(Cs1262FifoVal);

    return HDF_SUCCESS;
}

static inline int32_t SetIER(uint16_t it, uint8_t status)
{
    return Cs1262WriteRegbit(CS1262_IER_REG, it, status);
}

static inline int32_t ClearIFR(uint16_t it)
{
    return Cs1262WriteRegbit(CS1262_IFR_REG, it, CS1262_REG_BIT_SET);
}

int32_t Cs1262SetOption(uint32_t option)
{
    HDF_LOGI("%s: cs1262 setOption :%d", __func__, option);
    return HDF_SUCCESS;
}

static int32_t Writefw(Cs1262RegConfigTab *regTab)
{
    int32_t ret;
    Cs1262RegGroup regGroup[] = {
        { .regAddr = CS1262_CLOCK_REG, .regVal = regTab->clock, 1 },
        { .regAddr = CS1262_TL_BA, .regValGroup = regTab->tlTab, .regLen = TL_REGS_NUM },
        { .regAddr = CS1262_TX_BA, .regValGroup = regTab->txTab, .regLen = TX_REGS_NUM },
        { .regAddr = CS1262_RX_BA, .regValGroup = regTab->rxTab, .regLen = RX_REGS_NUM },
        { .regAddr = CS1262_TE_BA, .regValGroup = regTab->teTab, .regLen = TE_REGS_NUM },
        { .regAddr = CS1262_FIFO_OFFSET_REG, .regValGroup = (regTab->fifoTab + FIFO_WRITE_OFFSET),
          .regLen = (FIFO_REGS_NUM - FIFO_WRITE_OFFSET)}
    };

    if (RegLock(CS1262_REG_UNLOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: reg unlock failed", __func__);
        return HDF_FAILURE;
    }

    ret = Cs1262WriteGroup(regGroup, HDF_ARRAY_SIZE(regGroup));
    HDF_LOGI("%s: cs1262 init ret :%d", __func__, ret);

    if (RegLock(CS1262_REG_LOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: reg lock failed", __func__);
        return HDF_FAILURE;
    }
    return ret;
}

int32_t Cs1262SetMode(uint32_t mode)
{
    int32_t ret;
    Cs1262RegConfigTab *regTab = NULL;
    struct Cs1262DrvData *drvData = GetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if ((mode == NONE_MODE) || (drvData->regMode == mode)) {
        HDF_LOGI("%s: mode = %d, drvData->regMode = %d", __func__, mode, drvData->regMode);
        drvData->regMode = mode;
        return HDF_SUCCESS;
    }

    ret = Cs1262Loadfw(mode, &regTab);
    if ((ret != HDF_SUCCESS) || (regTab == NULL)) {
        HDF_LOGE("%s: Cs1262Loadfw failed", __func__);
        return HDF_FAILURE;
    }

    ret = Writefw(regTab);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Writefw failed", __func__);
        return HDF_FAILURE;
    }

    drvData->regMode = mode;

    HDF_LOGI("%s: set mode success", __func__);

    return HDF_SUCCESS;
}

int32_t Cs1262ReadData(uint8_t *outBuf, uint16_t outBufMaxLen, uint16_t *outLen)
{
    uint8_t ret;
    uint16_t ifr = 0;
    CHECK_NULL_PTR_RETURN_VALUE(outBuf, HDF_ERR_INVALID_PARAM);

    ret = Cs1262ReadRegs(CS1262_IFR_REG, &ifr, 1);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Cs1262ReadRegs CS1262_IFR_REG fail", __func__);
        return HDF_FAILURE;
    }

    if (ifr & IFR_RDY_FLAG) {
        ret = ReadFifo(outBuf, outBufMaxLen, outLen);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: ReadFifo fail", __func__);
        }

        ret = ClearIFR(FIFO_RDY_IFR_OFFSET);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: ClearIFR fail", __func__);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t Cs1262Enable()
{
    int32_t ret;
    struct Cs1262DrvData *drvData = GetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->regMode == NONE_MODE) {
        HDF_LOGW("%s: drvData->regMode == NONE_MODE, need set default mode when enable", __func__);
        if (Cs1262SetMode(DEFAULT_MODE) != HDF_SUCCESS) {
            HDF_LOGE("%s: set default mode failed", __func__);
            return HDF_FAILURE;
        }
    }

    if (RegLock(CS1262_REG_UNLOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: reg unlock failed", __func__);
        return HDF_FAILURE;
    }

    ret = SetIER(FIFO_RDY_IER_OFFSET, CS1262_REG_BIT_SET);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Cs1262Enable fail", __func__);
        return HDF_FAILURE;
    }

    ret = ResetModule(CS1262_TE_RST_OFFSET);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: reset failed", __func__);
        return HDF_FAILURE;
    }

    ret = Cs1262WriteRegbit(CS1262_TE_CTRL_REG, PRF_START_BIT, CS1262_REG_BIT_SET);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Cs1262WriteRegbit CS1262_TE_CTRL_REG fail", __func__);
        return HDF_FAILURE;
    }

    if (RegLock(CS1262_REG_LOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: reg lock failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t Cs1262Disable()
{
    int32_t ret;

    if (RegLock(CS1262_REG_UNLOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: reg unlock failed", __func__);
        return HDF_FAILURE;
    }

    ret = SetIER(FIFO_RDY_IER_OFFSET, CS1262_REG_BIT_RESET);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: SetIER fail", __func__);
        return HDF_FAILURE;
    }

    ret = Cs1262WriteRegbit(CS1262_TE_CTRL_REG, PRF_START_BIT, CS1262_REG_BIT_RESET);
    HDF_LOGI("%s: Cs1262 disable ret = %d", __func__, ret);

    if (RegLock(CS1262_REG_LOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: reg unlock failed", __func__);
        return HDF_FAILURE;
    }

    return ret;
}

static int32_t InitChip()
{
    (void)Cs1262SetMode(NONE_MODE);
    return ResetChip();
}

static int32_t CheckChipId(struct PpgCfgData *cfgData)
{
    uint16_t regRead = 0;
    uint8_t cnt = 0;

    uint16_t reg = cfgData->sensorCfg.sensorAttr.chipIdReg;
    uint16_t val = cfgData->sensorCfg.sensorAttr.chipIdValue;

    // retry 3 times
    while (cnt++ < 3) {
        if ((Cs1262ReadRegs(reg, &regRead, 1) == HDF_SUCCESS) && (regRead == val)) {
            HDF_LOGI("%s: cs1262 read chip id success!", __func__);
            return HDF_SUCCESS;
        }
    }

    HDF_LOGE("%s: cs1262 read chip id fail, reg=%u!", __func__, reg);
    return HDF_FAILURE;
}

static int32_t DispatchCs1262(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t Cs1262BindDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct Cs1262DrvData *drvData = (struct Cs1262DrvData *)OsalMemCalloc(sizeof(*drvData));
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_MALLOC_FAIL);
    (void)memset_s(drvData, sizeof(struct Cs1262DrvData), 0, sizeof(struct Cs1262DrvData));

    drvData->ioService.Dispatch = DispatchCs1262;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_cs1262DrvData = drvData;

    return HDF_SUCCESS;
}

int32_t Cs1262InitDriver(struct HdfDeviceObject *device)
{
    int32_t ret;
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    struct Cs1262DrvData *drvData = (struct Cs1262DrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    ret = ParsePpgCfgData(device->property, &(drvData->ppgCfg));
    if ((ret != HDF_SUCCESS) || (drvData->ppgCfg == NULL)) {
        HDF_LOGE("%s: cs1262 construct fail!", __func__);
        return HDF_FAILURE;
    }

    ret = Cs1262InitSpi(&drvData->ppgCfg->sensorCfg.busCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: cs1262 init spi!", __func__);
        return HDF_FAILURE;
    }

    ret = CheckChipId(drvData->ppgCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: cs1262 check chip fail!", __func__);
        return HDF_FAILURE;
    }

    ret = InitChip();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: cs1262 init chip fail!", __func__);
        return HDF_FAILURE;
    }

    struct PpgChipData chipData = {
        .cfgData = drvData->ppgCfg,
        .opsCall = {
            .ReadData = Cs1262ReadData,
            .Enable = Cs1262Enable,
            .Disable = Cs1262Disable,
            .SetOption = Cs1262SetOption,
            .SetMode = Cs1262SetMode,
        }
    };

    ret = RegisterPpgChip(&chipData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Register CS1262 failed", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: cs1262 init driver success", __func__);
    return HDF_SUCCESS;
}

void Cs1262ReleaseDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN(device);
    struct Cs1262DrvData *drvData = (struct Cs1262DrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);
    CHECK_NULL_PTR_RETURN(drvData->ppgCfg);

    Cs1262ReleaseSpi(&drvData->ppgCfg->sensorCfg.busCfg);
    (void)memset_s(drvData, sizeof(struct Cs1262DrvData), 0, sizeof(struct Cs1262DrvData));
    OsalMemFree(drvData);
}

struct HdfDriverEntry g_ppgCs1262DevEntry = {
    .moduleVersion = CS1262_MODULE_VER,
    .moduleName = "HDF_SENSOR_PPG_CS1262",
    .Bind = Cs1262BindDriver,
    .Init = Cs1262InitDriver,
    .Release = Cs1262ReleaseDriver,
};

HDF_INIT(g_ppgCs1262DevEntry);
