/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

/**
 * @addtogroup I3C
 * @{
 *
 * @brief Provides Improved Inter-Integrated Circuit (I3C) interfaces.
 *
 * This module allows a driver to perform operations on an I3C controller for accessing devices on the I3C bus,
 * including creating and destroying I3C controller handles as well as reading and writing data.
 *
 * @since 1.0
 */

/**
 * @file i3c_if.h
 *
 * @brief Declares the standard I3C interface functions.
 *
 * @since 1.0
 */

#ifndef I3C_IF_H
#define I3C_IF_H

#include "hdf_platform.h"
#include "i3c_core.h"
#include "i3c_ccc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

enum TransMode {
    I2C_MODE = 0,
    I3C_MODE,
    HDR_MODE,
    CCC_CMD_MODE,
};

struct I3cMsg {
    struct CccCmd *cmd;
    uint16_t addr;
    uint8_t *buf;
    uint16_t len;
    uint16_t flags;
    enum TransMode mode;
    struct I3cCccCmd ccc;
    uint16_t err;
};

enum I3cFlag {
    /** Read flag. The value <b>1</b> indicates the read operation, and <b>0</b> indicates the write operation. */
    I3C_FLAG_READ           = (0x1 << 0),
    /** Non-ACK read flag. The value <b>1</b> indicates that no ACK signal is sent during the read process. */
    I3C_FLAG_READ_NO_ACK    = (0x1 << 11),
    /** Ignoring no-ACK flag. The value <b>1</b> indicates that the non-ACK signal is ignored. */
    I3C_FLAG_IGNORE_NO_ACK  = (0x1 << 12),
    /**
     * No START condition flag. The value <b>1</b> indicates that there is no START condition for the message
     * transfer.
     */
    I3C_FLAG_NO_START       = (0x1 << 14),
    /** STOP condition flag. The value <b>1</b> indicates that the current transfer ends with a STOP condition. */
    I3C_FLAG_STOP           = (0x1 << 15),
};

typedef int32_t (*I3cIbiFunc)(uint16_t addr, void *data);

/**
 * @brief Obtains the handle of an I3C controller.
 *
 * You must call this function before accessing the I3C bus.
 *
 * @param number Indicates the I3C controller ID.
 *
 * @return Returns the pointer to the {@link DevHandle} of the I3C controller if the operation is successful;
 * returns <b>NULL</b> otherwise.
 * @since 1.0
 */
DevHandle I3cOpen(int16_t number);

 /**
 * @brief Releases the handle of an I3C controller.
 *
 * If you no longer need to access the I3C controller, you should call this function to close its handle so as
 * to release unused memory resources.
 *
 * @param handle Indicates the pointer to the device handle of the I3C controller.
 *
 * @since 1.0
 */
void I3cClose(DevHandle handle);

 /**
 * @brief Launches an transfer to an I3C device or a compatible I2C device,
 *         or send a CCC (common command code) to an I3C device which is supported.
 *
 * @param handle Indicates the pointer to the device handle of the I3C controller obtained via {@link I2cOpen}.
 * @param msgs Indicates the pointer to the I3C transfer message structure array.
 * @param count Indicates the length of the message structure array.
 *
 * @return Returns the number of transferred message structures if the operation is successful;
 * returns a negative value otherwise.
 * @see I3cMsg
 * @attention This function does not limit the number of message structures specified by <b>count</b> or the data
 * length of each message structure. The specific I3C controller determines the maximum number and data length allowed.
 *
 * @since 1.0
 */
int32_t I3cTransfer(DevHandle handle, struct I3cMsg *msg, int16_t count, enum TransMode mode);
 /**
 * @brief Launches an transfer to an I3C device or a compatible I2C device,
 *         or send a CCC (common command code) to an I3C device which is supported.
 *
 * @param handle Indicates the pointer to the device handle of the I3C controller obtained via {@link I2cOpen}.
 *
 * @return Returns 0 if the operation is successful; Returns a negative value otherwise.
 *
 * @since 1.0
 */
int32_t I3cDaa(DevHandle handle);
 /**
 * @brief Launches an transfer to an I3C device or a compatible I2C device,
 *         or send a CCC (Common Command Code) to an I3C device which is supported.
 *
 * @param handle Indicates the pointer to the device handle of the I3C controller obtained via {@link I2cOpen}.
 * @param addr Indicates the address of device to requeset IBI(In-Band Interrupt).
 * @param func Indicates the call back function of the IBI.
 * @param payload Indicates the length of payload data, in bytes.
 *
 * @return Returns 0 if the operation is successful; Returns a negative value otherwise.
 *
 * @since 1.0
 */
int32_t I3cRequestIbi(DevHandle handle, uint16_t addr, I3cIbiFunc func, uint32_t payload);

 /**
 * @brief Launches an transfer to an I3C device or a compatible I2C device,
 *         or send a CCC (Common Command Code) to an I3C device which is supported.
 *
 * @param handle Indicates the pointer to the device handle of the I3C controller obtained via {@link I2cOpen}.
 * @param addr Indicates the address of device to free IBI.
 *
 * @return Returns 0 if the operation is successful; Returns a negative value otherwise.
 *
 * @since 1.0
 */
int32_t I3cFreeIbi(DevHandle handle, uint16_t addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* I3C_IF_H */
/** @} */
