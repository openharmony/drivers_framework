/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

/**
 * @addtogroup REGULATOR
 * @{
 *
 * @Brief Provides regulator APIs, such as enabling and setting voltage / current.
 *
 * The REGULATOR module abstracts the regulator functions of different system platforms and provides stable APIs for drivers.
 * You can use this module to create / release the regulator device handle, enable regulator, set voltage, current, disable, etc.
 *
 * @since 1.0
 */

/**
 * @file regulator_if.h
 *
 * @brief Declares standard regulator APIs.
 *
 * @since 1.0
 */

#ifndef REGULATOR_IF_H
#define REGULATOR_IF_H

#include "hdf_platform.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/**
 * @brief Enumerates regulator disableMode.
 *
 * To disable the mode using the regulator, call the {@ link RegulatorDisable} function.
 *
 * @since 1.0
 */

enum RegulatorDisableMode {
    NORMAL_DISABLE,
    DEFERRED_DISABLE,
    FORCE_DISABLE,
    MAX_DISABLE_MODE,
};

/**
 * @brief Gets a regulator.
 *
 * This function must be called to get its device handle before operating the regulator.
 *
 * @param name Indicates regulator name.
 *
 * @return If the operation is successful, a pointer to the regulator device handle is returned.
 *
 * @since 1.0
 */
DevHandle RegulatorGet(const char *name);

/**
 * @brief Releases a regulator.
 *
 * If you no longer need the regulator, call this function to turn it off and release its device handle to prevent.
 * Use memory resources unnecessarily.
 *
 * @param handle Represents a pointer to the regulator device handle.
 *
 * @since 1.0
 */
void RegulatorPut(DevHandle handle);

/**
 * @brief Enables a regulator.
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @return <b>0</b> If the regulator enables successfully; Otherwise, a negative value is returned.
 *
 * @attention That if the regulator has been enabled before calling this function, calling this function will succeed.
 * 
 * @since 1.0
 */
int32_t RegulatorEnable(DevHandle handle);

/**
 * @brief Disable a regulator.
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @param disableMode  There are three disabled modes.
 * @return <b>0</b> If the regulator disable successfully; Otherwise, a negative value is returned.
 *
 * @attention If the regulator device AlwaysOn is true, disabling may fail, depending on the specific mode.
 * 
 * @since 1.0
 */
int32_t RegulatorDisable(DevHandle handle,int32_t disableMode);

/**
 * @brief Determine whether a regulator is enabled.
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @return <b>0</b> If the regulator isEnabled successfully; Otherwise, a negative value is returned.
 *
 * @since 1.0
 */
int32_t RegulatorIsEnabled(DevHandle handle);

/**
 * @brief Set the voltage value of a regulator.
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @return <b>0</b> If the regulator setVoltage successfully; Otherwise, a negative value is returned.
 *
 * @attention If the set voltage is not within the range, the setting fails.
 * 
 * @since 1.0
 */
int32_t RegulatorSetVoltage(DevHandle handle, int32_t voltage);

/**
 * @brief Get a regulator voltage.
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @param voltage Voltage obtained.
 * @return <b>0</b> If the regulator get voltage successfully; Otherwise, a negative value is returned.
 * 
 * @since 1.0
 */
int32_t RegulatorGetVoltage(DevHandle handle, int32_t *voltage);

/**
 * @brief Set regulator voltage range
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @param vmin Minimum value of regulator voltage
 * @param vmax Maximum regulator voltage
 * @return <b>0</b> If the regulator set voltage range successfully; Otherwise, a negative value is returned.
 *
 * @attention If the setting range exceeds the limit, the setting fails
 * 
 * @since 1.0
 */
int32_t RegulatorSetVoltageRange(DevHandle handle, int32_t vmin, int32_t vmax);

/**
 * @brief Set the current value of a regulator
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @return <b>0</b> If the regulator setCurrent successfully; Otherwise, a negative value is returned.
 *
 * @attention If the set current is not within the range, the setting fails
 * 
 * @since 1.0
 */
int32_t RegulatorSetCurrent(DevHandle handle, int32_t current);

/**
 * @brief Get a regulator current
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @param voltage Current obtained
 * @return <b>0</b> If the regulator getCurrent successfully; Otherwise, a negative value is returned.
 * 
 * @since 1.0
 */
int32_t RegulatorGetCurrent(DevHandle handle, int32_t *current);

/**
 * @brief Set regulator current range
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @param cmin Minimum value of regulator current
 * @param cmax Maximum regulator current
 * @return <b>0</b> If the regulator set current range successfully; Otherwise, a negative value is returned.
 *
 * @attention If the setting range exceeds the limit, the setting fails
 * 
 * @since 1.0
 */
int32_t RegulatorSetCurrentRange(DevHandle handle, int32_t cmin, int32_t cmax);

/**
 * @brief Get a regulator status
 *
 * @param handle  Represents a pointer to the regulator handle, which is obtained through {@ link RegulatorGet}.
 * @param status Status obtained
 * @return <b>0</b> If the regulator get status successfully; Otherwise, a negative value is returned.
 * 
 * @since 1.0
 */
int32_t RegulatorGetStatus(DevHandle handle, int32_t *status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* REGULATOR_IF_H */
/** @} */
