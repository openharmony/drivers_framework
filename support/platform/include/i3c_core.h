/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef I3C_CORE_H
#define I3C_CORE_H

#include "i3c_if.h"
#include "i3c_ccc.h"
#include "hdf_base.h"
#include "hdf_dlist.h"
#include "osal_spinlock.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define I3C_BUS_MAX              16
#define I3C_ADDR_MAX             127
#define I3C_IBI_MAX              10
#define ADDRS_STATUS_BITS        2
#define BITS_PER_UINT16          16
#define ADDRS_PER_UINT16         8

#define ADDR_STATUS_BIT0_MASK    0x1
#define ADDR_STATUS_BIT1_MASK    0x2
#define ADDR_STATUS_MASK         0x3

struct I3cCntlr;
struct I3cMethod;
struct I3cLockMethod;
struct I3cDevice;
struct I3cVendor;
struct I3cConfig;

enum I3cBusMode {
    I3C_BUS_SDR_MODE = 0,
    I3C_BUS_HDR_MODE,
};

enum I3cDeviceType {
    I3C_CNTLR_I2C_DEVICE = 0,
    I3C_CNTLR_I3C_CONV_DEVICE,
    I3C_CNTLR_I3C_SM_DEVICE,     // secondary master I3C device
};

struct I3cConfig {
    enum I3cBusMode busMode;
    struct I3cDevice *curMaster;  
};

struct I3cCntlr {
    OsalSpinlock lock;
    void *owner;
    int16_t busId;
    struct I3cConfig config;
    uint16_t addrSlot[(I3C_ADDR_MAX + 1) / ADDRS_PER_UINT16];
    struct I3cIbiInfo *ibiSlot[I3C_IBI_MAX];
    const struct I3cMethod *ops;
    const struct I3cLockMethod *lockOps;
    void *priv;
};

struct I3cVendor {
    uint32_t vendor_mipi_id;
    uint32_t vendor_version_id;
    uint32_t vendor_product_id;
};

struct I3cIbiInfo {
    I3cIbiFunc irqFunc;
    int32_t payload;
    void *irqData;
};

struct I3cDevice {
    struct DListHead list;
    struct I3cCntlr *cntlr;
    uint16_t bcr;
    uint16_t dcr;
    uint64_t pid;
    uint16_t addr;
    enum I3cDeviceType type;
    uint16_t dynaAddr;
    uint16_t supportIbi;
    struct I3cIbiInfo *ibi;
    struct I3cVendor vendor;
    void *devPriv;
};

struct I3cMethod {
    int32_t (*doDaa)(struct I3cCntlr cntlr);

    int32_t (*sendCccCmd)(struct I3cCntlr *cntlr, struct I3cCccCmd ccc);

    int32_t (*Transfer)(struct I3cCntlr *cntlr, struct I3cMsg *msgs, int16_t count);

    int32_t (*i2cTransfer)(struct I3cCntlr *cntlr, struct I3cMsg *msgs, int16_t count);

    int32_t (*requestIbi)(struct I3cDevice *dev, I3cIbiFunc func, uint32_t payload);

    void (*freeIbi)(struct I3cDevice *dev);
};

struct I3cLockMethod {
    /**
     * @brief Get exclusive access to an I3C controller.
     *
     * @param cntlr Indicates the I3C controller to access.
     *
     * @return Returns 0 on success; returns a negative value otherwise.
     * @since 1.0
     */
    int32_t (*lock)(struct I3cCntlr *cntlr);
    /**
     * @brief Release exclusive access to an I3C controller.
     *
     * @param cntlr Indicates the I3C controller to release.
     *
     * @since 1.0
     */
    void (*unlock)(struct I3cCntlr *cntlr);
};

enum I3C_ERROR_CODE {
    I3C_ERROR_UNKNOWN = 0,
    I3C_ERROR_M0,
    I3C_ERROR_M1,
    I3C_ERROR_M2,
    I3C_ERROR_S0,
    I3C_ERROR_S1,
    I3C_ERROR_S2,
    I3C_ERROR_S3,
    I3C_ERROR_S4,
    I3C_ERROR_S5,
    I3C_ERROR_S6,
};

enum I3cAddrStatus {
    I3C_ADDR_FREE = 0,
    I3C_ADDR_RESERVED,
    I3C_ADDR_I2C_DEVICE,
    I3C_ADDR_I3C_DEVICE,
};

int32_t I3cCntlrGetConfig(struct I3cCntlr *cntlr, struct I3cConfig *config);

int32_t I3cCntlrSetConfig(struct I3cCntlr *cntlr, struct I3cConfig *config);

int32_t I3cCntlrRequestIbi(struct I3cCntlr *cntlr, uint16_t addr, I3cIbiFunc func, uint32_t payload);

int32_t I3cCntlrFreeIbi(struct I3cCntlr *cntlr, uint16_t addr);

int32_t I3cCntlrSendCccCmd(struct I3cCntlr *cntlr, struct I3cCccCmd ccc);

int32_t I3cCntlrI2cTransfer(struct I3cCntlr *cntlr, struct I3cMsg *msgs, int16_t count);

int32_t I3cCntlrTransfer(struct I3cCntlr *cntlr, struct I3cMsg *msgs, int16_t count);

int32_t I3cCntlrDaa(struct I3cCntlr *cntlr);

int32_t I3cDeviceAdd(struct I3cDevice *device);

void I3cDeviceRemove(struct I3cDevice *device);

struct I3cCntlr *I3cCntlrGet(int16_t number);

void I3cCntlrPut(struct I3cCntlr *cntlr);

int32_t I3cCntlrAdd(struct I3cCntlr *cntlr);

void I3cCntlrRemove(struct I3cCntlr *cntlr);

void I3cCntlrIrqCallback(struct I3cCntlr *cntlr, uint16_t addr);

enum I3cIoCmd {
    I3c_Io_I2cTransfer = 0,
    I3C_IO_PRIV_TRANSFER,
    I3C_IO_OPEN,
    I3C_IO_CLOSE,
    I3C_IO_GET_CONFIG,
    I3C_IO_SET_CONFIG,
    I3C_IO_REQUEST_IBI,
    I3C_IO_FREE_IBI,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* I3C_CORE_H */
