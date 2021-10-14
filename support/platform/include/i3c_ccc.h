/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef I3C_CCC_H
#define I3C_CCC_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct I3cCccCmd;

#define I3C_BROADCAST_ADDR    0x7e;

/* Broatcast commands */
#define I3C_CCC_ENEC_B        0x00
#define I3C_CCC_DISEC_B       0x01
#define I3C_CCC_ENTAS0_B      0x02
#define I3C_CCC_ENTAS1_B      0x03
#define I3C_CCC_ENTAS2_B      0x04
#define I3C_CCC_ENTAS3_B      0x05
#define I3C_CCC_RSTDAA_B      0x06
#define I3C_CCC_ENTDAA        0x07
#define I3C_CCC_DEFSLVS       0x08
#define I3C_CCC_SETMWL_B      0x09
#define I3C_CCC_SETMRL_B      0x0a
#define I3C_CCC_ENTTM         0x0b
#define I3C_CCC_ENDXFER       0X12
#define I3C_CCC_ENTHDR0       0x20
#define I3C_CCC_ENTHDR1       0x21
#define I3C_CCC_ENTHDR2       0x22
#define I3C_CCC_ENTHDR3       0x23
#define I3C_CCC_ENTHDR4       0x24
#define I3C_CCC_ENTHDR5       0x25
#define I3C_CCC_ENTHDR6       0x26
#define I3C_CCC_ENTHDR7       0x27
#define I3C_CCC_SETXTIME_B    0x28
#define I3C_CCC_SETAASA       0X29
#define I3C_CCC_VENDOR_B      0x61
/* Driect commands */
#define I3C_CCC_ENEC_D        0x80
#define I3C_CCC_DISEC_D       0x81
#define I3C_CCC_ENTAS0_D      0x82
#define I3C_CCC_ENTAS1_D      0x83
#define I3C_CCC_ENTAS2_D      0x84
#define I3C_CCC_ENTAS3_D      0x85
#define I3C_CCC_RSTDAA_D      0x86
#define I3C_CCC_SETDASA       0x87
#define I3C_CCC_SETNEWDA      0x88
#define I3C_CCC_SETMWL_D      0x89
#define I3C_CCC_SETMRL_D      0x8a
#define I3C_CCC_GETMWL        0x8b
#define I3C_CCC_GETMRL        0x8c
#define I3C_CCC_GETPID        0x8d
#define I3C_CCC_GETBCR        0x8e
#define I3C_CCC_GETDCR        0x8f
#define I3C_CCC_GETSTATUS     0x90
#define I3C_CCC_GETACCMST     0x91
#define I3C_CCC_SETBRGTGT     0x93
#define I3C_CCC_GETMXDS       0x94
#define I3C_CCC_GETHDRCAP     0x95
#define I3C_CCC_SETXTIME_D    0x98
#define I3C_CCC_GETXTIME      0x99
#define I3C_CCC_VENDOR_D      0xe0

struct I3cCccCmd {
    uint8_t cmd;
    uint16_t dest;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* I3C_CORE_H */
