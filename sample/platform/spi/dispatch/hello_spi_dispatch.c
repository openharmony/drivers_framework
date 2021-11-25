/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "spi_if.h"
#include "hdf_log.h"

#define HDF_LOG_TAG hello_spi_dispatch

#define SPI_BUS_NUM       3
#define SPI_MSG_SPEED     115200

int main()
{
    int32_t ret;
    struct SpiDevInfo spiDevInfo;       /* SPI device descriptor */
    struct DevHandle *spiHandle = NULL; /* SPI device handle */
    spiDevInfo.busNum = SPI_BUS_NUM;              /* SPI device bus number */
    spiDevInfo.csNum = 0;               /* SPI device CS number */
    spiHandle = SpiOpen(&spiDevInfo);
    if (spiHandle == NULL) {
        HDF_LOGE("SpiOpen failed");
        return HDF_FAILURE;
    }

    uint8_t wbuff[1] = {0x12};
    uint8_t rbuff[1] = {0};
    struct SpiMsg msg;        /* Custom message to be transferred */
    msg.wbuf = wbuff;         /* Pointer to the data to write */
    msg.rbuf = rbuff;         /* Pointer to the data to read */
    msg.len = 1;              /* The length of the data to be read or written is 1 bits. */
    msg.csChange = 1;         /* Disable the CS before the next transfer. */
    msg.delayUs = 0;          /* No delay before the next transfer */
    msg.speed = SPI_MSG_SPEED;       /* Speed of this transfer */
    ret = SpiTransfer(spiHandle, &msg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("SpiTransfer failed, ret %d", ret);
        return ret;
    }
    SpiClose(spiHandle);
    return ret;
}