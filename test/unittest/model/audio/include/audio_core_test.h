/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CORE_TEST_H
#define AUDIO_CORE_TEST_H

#include "hdf_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t AudioSocTestRegisterPlatform(void);
int32_t AudioSocTestRegisterDai(void);
int32_t AudioTestRegisterAccessory(void);
int32_t AudioTestRegisterCodec(void);
int32_t AudioTestRegisterDsp(void);
int32_t AudioTestSocDeviceRegister(void);
int32_t AudioTestBindDaiLink(void);
int32_t AudioTestUpdateCodecRegBits(void);
int32_t AudioTestUpdateAccessoryRegBits(void);
int32_t AudioTestKcontrolGetCodec(void);
int32_t AudioTestKcontrolGetAccessory(void);
int32_t AudioTestAddControl(void);
int32_t AudioTestAddControls(void);
int32_t AudioTestCodecReadReg(void);
int32_t AudioTestAccessoryReadReg(void);
int32_t AudioTestCodecWriteReg(void);
int32_t AudioTestAccessoryWriteReg(void);
int32_t AudioTestInfoCtrlOps(void);
int32_t AudioTestCodecGetCtrlOps(void);
int32_t AudioTestAccessoryGetCtrlOps(void);
int32_t AudioTestCodecSetCtrlOps(void);
int32_t AudioTestAccessorySetCtrlOps(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_CORE_TEST_H */
