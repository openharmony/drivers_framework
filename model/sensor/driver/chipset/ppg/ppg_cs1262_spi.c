/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ppg_cs1262_spi.h"
#include "spi_if.h"

#define HDF_LOG_TAG hdf_sensor_cs1262_spi

#define CS1262_HEAD_BUF_SINGLEREG_LEN 4
#define CS1262_HEAD_BUF_FULL_LEN 6

static struct SensorBusCfg *g_busCfg = NULL;

static inline uint16_t ReverseEndianInt16(uint16_t data)
{
    return (((data & 0x00FF) << 8) |
            ((data & 0xFF00) >> 8));
}

static inline uint16_t Cs1262RegAddrConvert(uint16_t reg)
{
    /* CS1262 takes the register address from bit[2] to bit[15], bit[1~0] fixed as 00,
     * so reg needs to be shifted left by 2 bits to convert it to the address format required by CS1262
     */
    return (reg << 2) & 0x0FFC;
}

static inline uint8_t Cs1262GetHighByteInt16(uint16_t data)
{
    return (data & 0xFF00) >> 8;
}

static inline uint8_t Cs1262GetLowByteInt16(uint16_t data)
{
    return data & 0x00FF;
}

void Cs1262ReleaseSpi(struct SensorBusCfg *busCfg)
{
    SpiClose(busCfg->spiCfg.handle);
    busCfg->spiCfg.handle = NULL;
    g_busCfg = NULL;
}

int32_t Cs1262InitSpi(struct SensorBusCfg *busCfg)
{
    int32_t ret;
    CHECK_NULL_PTR_RETURN_VALUE(busCfg, HDF_ERR_INVALID_PARAM);
    struct SpiDevInfo spiDevinfo = {
        .busNum = busCfg->spiCfg.busNum,
        .csNum = busCfg->spiCfg.csNum,
    };

    HDF_LOGI("%s: SpiOpen busNum = %d, csNum = %d", __func__, spiDevinfo.busNum, spiDevinfo.csNum);

    busCfg->spiCfg.handle = SpiOpen(&spiDevinfo);
    if (busCfg->spiCfg.handle == NULL) {
        HDF_LOGE("%s: SpiOpen failed", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGD("%s: SpiSetCfg: maxSpeedHz:%d, mode=%d, transferMode=%d, bitsPerWord=%d",
        __func__, busCfg->spiCfg.spi.maxSpeedHz, busCfg->spiCfg.spi.mode,
        busCfg->spiCfg.spi.transferMode, busCfg->spiCfg.spi.bitsPerWord);

    ret = SpiSetCfg(busCfg->spiCfg.handle, &busCfg->spiCfg.spi);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: SpiSetCfg failed", __func__);
        SpiClose(busCfg->spiCfg.handle);
        return ret;
    }

    g_busCfg = busCfg;
    return HDF_SUCCESS;
}

int32_t Cs1262ReadFifoReg(Cs1262FifoVal *fifoBuf, uint16_t fifoLen)
{
    CHECK_NULL_PTR_RETURN_VALUE(fifoBuf, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(g_busCfg, HDF_ERR_NOT_SUPPORT);

    uint8_t headBuf[CS1262_HEAD_BUF_FULL_LEN] = {
        CS1262_SPI_NOCHECK_CONTINUOUSREAD,
        0,
        Cs1262GetHighByteInt16(Cs1262RegAddrConvert(CS1262_FIFO_DATA_REG)),
        Cs1262GetLowByteInt16(Cs1262RegAddrConvert(CS1262_FIFO_DATA_REG)),
        Cs1262GetHighByteInt16(fifoLen - 1),
        Cs1262GetLowByteInt16(fifoLen - 1),
    };

    struct SpiMsg msg[] = {
        {
            .wbuf = headBuf,
            .rbuf = NULL,
            .len = CS1262_HEAD_BUF_FULL_LEN,
            .keepCs = 0,  // cs low
            .delayUs = 0,
            .speed = g_busCfg->spiCfg.spi.maxSpeedHz,
        },
        {
            .wbuf = NULL,
            .rbuf = (uint8_t *)fifoBuf,
            .len = fifoLen * sizeof(Cs1262FifoVal),
            .keepCs = 1,  // cs high
            .delayUs = 0,
            .speed = g_busCfg->spiCfg.spi.maxSpeedHz
        }
    };

    if (SpiTransfer(g_busCfg->spiCfg.handle, msg, HDF_ARRAY_SIZE(msg)) != HDF_SUCCESS) {
        HDF_LOGE("%s: Read Fifo data use spi failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t Cs1262ReadRegs(uint16_t regAddr, uint16_t *dataBuf, uint16_t dataLen)
{
    CHECK_NULL_PTR_RETURN_VALUE(dataBuf, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(g_busCfg, HDF_ERR_NOT_SUPPORT);

    uint8_t headBuf[CS1262_HEAD_BUF_FULL_LEN] = {
        CS1262_SPI_NOCHECK_SINGLEREAD,
        0,
        Cs1262GetHighByteInt16(Cs1262RegAddrConvert(regAddr)),
        Cs1262GetLowByteInt16(Cs1262RegAddrConvert(regAddr))
    };
    uint8_t bufLen = CS1262_HEAD_BUF_SINGLEREG_LEN;

    if (dataLen > 1) {
        headBuf[0] = CS1262_SPI_NOCHECK_CONTINUOUSREAD;
        headBuf[bufLen++] = Cs1262GetHighByteInt16(dataLen - 1);
        headBuf[bufLen++] = Cs1262GetLowByteInt16(dataLen - 1);
    }

    struct SpiMsg msg[] = {
        {
            .wbuf = headBuf,
            .rbuf = NULL,
            .len = bufLen,
            .keepCs = 0,
            .delayUs = 0,
            .speed = g_busCfg->spiCfg.spi.maxSpeedHz
        },
        {
            .wbuf = NULL,
            .rbuf = (uint8_t *)dataBuf,
            .len = dataLen * 2,  // 2 bytes reg
            .keepCs = 1,  // 1 means enable
            .delayUs = 0,
            .speed = g_busCfg->spiCfg.spi.maxSpeedHz
        }
    };

    if (SpiTransfer(g_busCfg->spiCfg.handle, msg, HDF_ARRAY_SIZE(msg)) != HDF_SUCCESS) {
        HDF_LOGE("%s: Read data use spi failed", __func__);
        return HDF_FAILURE;
    }

    for (uint16_t index = 0; index < dataLen; index++) {
        *(dataBuf + index) = ReverseEndianInt16(*(dataBuf + index));
    }

    return HDF_SUCCESS;
}

int32_t Cs1262WriteRegs(uint16_t regAddr, uint16_t *dataBuf, uint16_t dataLen)
{
    CHECK_NULL_PTR_RETURN_VALUE(dataBuf, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(g_busCfg, HDF_ERR_NOT_SUPPORT);

    uint8_t headBuf[CS1262_HEAD_BUF_FULL_LEN] = {
        CS1262_SPI_NOCHECK_SINGLEWRITE,
        0,
        Cs1262GetHighByteInt16(Cs1262RegAddrConvert(regAddr)),
        Cs1262GetLowByteInt16(Cs1262RegAddrConvert(regAddr))
    };
    uint8_t bufLen = CS1262_HEAD_BUF_SINGLEREG_LEN;

    if (dataLen > 1) {
        headBuf[0] = CS1262_SPI_NOCHECK_CONTINUOUSWRITE;
        headBuf[bufLen++] = Cs1262GetHighByteInt16(dataLen - 1);
        headBuf[bufLen++] = Cs1262GetLowByteInt16(dataLen - 1);
    }

    for (uint16_t index = 0; index < dataLen; index++) {
        *(dataBuf + index) = ReverseEndianInt16(*(dataBuf + index));
    }

    struct SpiMsg msg[] = {
        {
            .wbuf = headBuf,
            .rbuf = NULL,
            .len = bufLen,
            .keepCs = 0,
            .delayUs = 0,
            .speed = g_busCfg->spiCfg.spi.maxSpeedHz
        },
        {
            .wbuf = (uint8_t *)dataBuf,
            .rbuf = NULL,
            .len = dataLen * 2,  // 1 data has 2 bytes
            .keepCs = 1,  // 1 means enable
            .delayUs = 0,
            .speed = g_busCfg->spiCfg.spi.maxSpeedHz
        }
    };

    if (SpiTransfer(g_busCfg->spiCfg.handle, msg, HDF_ARRAY_SIZE(msg)) != HDF_SUCCESS) {
        HDF_LOGE("%s: Write data use spi failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

inline int32_t Cs1262WriteReg(uint16_t regAddr, uint16_t data)
{
    return Cs1262WriteRegs(regAddr, &data, 1);
}

int32_t Cs1262WriteData(uint8_t *data, uint16_t dataLen)
{
    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(g_busCfg, HDF_ERR_NOT_SUPPORT);
    struct SpiMsg msg = {
        .wbuf = data,
        .rbuf = NULL,
        .len = dataLen,
        .keepCs = 1,  // 1 means enable
        .delayUs = 0,
        .speed = g_busCfg->spiCfg.spi.maxSpeedHz
    };
    return SpiTransfer(g_busCfg->spiCfg.handle, &msg, 1);
}

int32_t Cs1262WriteRegbit(uint16_t regAddr, uint16_t setbit, Cs1262BitStatus bitval)
{
    uint16_t regData = 0;

    if (Cs1262ReadRegs(regAddr, &regData, 1) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed", __func__);
        return HDF_FAILURE;
    }

    if (bitval == CS1262_REG_BIT_SET) {
        regData |= (uint16_t)(1 << setbit);
    } else {
        regData &=  (uint16_t)(~(1 << setbit));
    }

    return Cs1262WriteRegs(regAddr, &regData, 1);
}

int32_t Cs1262WriteGroup(Cs1262RegGroup *regGroup, uint16_t groupLen)
{
    int32_t ret;

    CHECK_NULL_PTR_RETURN_VALUE(regGroup, HDF_ERR_INVALID_PARAM);

    for (uint16_t index = 0; index < groupLen; index++) {
        if (regGroup[index].regLen == 1) {
            ret = Cs1262WriteReg(regGroup[index].regAddr, regGroup[index].regVal);
        } else {
            ret = Cs1262WriteRegs(regGroup[index].regAddr, regGroup[index].regValGroup, regGroup[index].regLen);
        }

        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: failed, index = %u", __func__, index);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}
