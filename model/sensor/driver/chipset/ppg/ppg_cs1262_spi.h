/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PPG_CS1262_SPI_H
#define PPG_CS1262_SPI_H

#include "hdf_types.h"
#include "sensor_platform_if.h"
/***************************************** TYPEDEF ******************************************/
typedef enum {
    CS1262_REG_BIT_RESET = 0,
    CS1262_REG_BIT_SET = 1,
} Cs1262BitStatus;

typedef struct {
    uint16_t regAddr;
    union {
        uint16_t regVal;
        uint16_t *regValGroup;
    };
    uint16_t regLen;
} Cs1262RegGroup;

typedef struct {
    uint32_t reserve : 2;
    uint32_t adc_data : 22;
    uint32_t tl : 2;
    uint32_t rx : 2;
    uint32_t phaseGroup : 4;
} Cs1262FifoVal;

/****************************************** DEFINE ******************************************/
// lock reg
#define CS1262_LOCK 0x0000
// unlock reg
#define CS1262_UN_LOCK1 0x0059
#define CS1262_UN_LOCK2 0x0016
#define CS1262_UN_LOCK3 0x0088

/****************************************** OFFSET ******************************************/
// PRF
#define PRF_START_BIT (0x0000)
// REG WR PROT
#define LOCK_REG_OFFSET (0x01u << 0)

// RESET CON
#define FIFORST_REG_OFFSET (0x01u << 1)
#define TERST_REG_OFFSET   (0x01u << 2)
#define ADCRST_REG_OFFSET  (0x01u << 3)

// IER IFR
#define INT_MODE_TRIGER (0 << 15) // 0 triger

#define LED_WARN_IER_OFFSET 5
#define REG_ERR_IER_OFFSET  4
#define TS_RDY_IER_OFFSET   3
#define DATA_RDY_IER_OFFSET 2
#define THR_DET_IER_OFFSET  1
#define FIFO_RDY_IER_OFFSET 0

#define LED_WARN_IFR_OFFSET 5
#define REG_ERR_IFR_OFFSET  4
#define TS_RDY_IFR_OFFSET   3
#define DATA_RDY_IFR_OFFSET 2
#define THR_DET_IFR_OFFSET  1
#define FIFO_RDY_IFR_OFFSET 0

#define IFR_RDY_FLAG 0x0001
// TE CTRL
#define PRF_START_OFFSET (0x01u << 0)

// FIFO STATE
#define FIFO_FULL_OFFSET  (0x01u << 12)
#define FIFO_EMPTY_OFFSET (0x01u << 11)
#define FIFO_NUM_OFFSET   0x3FF

// RESET OFFSET
#define CS1262_FIFO_RST_OFFSET 1
#define CS1262_TE_RST_OFFSET   2
#define CS1262_ADC_RST_OFFSET  3

/******************************************* REGS *******************************************/
// SYS_BA
#define CS1262_SYS_BA         (0x0000)
#define CS1262_WRPROT_REG     (CS1262_SYS_BA + 0x00)
#define CS1262_CLOCK_REG      (CS1262_SYS_BA + 0x01)
#define CS1262_RSTCON_REG     (CS1262_SYS_BA + 0x02)
#define CS1262_IER_REG        (CS1262_SYS_BA + 0x03)
#define CS1262_IFR_REG        (CS1262_SYS_BA + 0x04)
#define CS1262_SYS_STATE_REG  (CS1262_SYS_BA + 0x05)
// TL_BA
#define CS1262_TL_BA          (0x0010)
// TX_BA
#define CS1262_TX_BA          (0x0050)
// RX_BA
#define CS1262_RX_BA          (0x0070)
// TE_BA
#define CS1262_TE_BA          (0x00E0)
#define CS1262_TE_CTRL_REG    (CS1262_TE_BA + 0x00)
// TE_BA
#define CS1262_WEAR_BA         (0x0120)
// FIFO & ADC
#define CS1262_ADC_BA          (0x140)
#define CS1262_FIFO_DATA_REG   (CS1262_ADC_BA + 0x00)
#define CS1262_FIFO_STATE_REG  (CS1262_ADC_BA + 0x01)
#define CS1262_FIFO_OFFSET_REG (CS1262_ADC_BA + 0x02)
// ID_BA
#define CS1262_ID_BA           (0x1F0)
#define CS1262_CHIP_ID1_REG    (CS1262_ID_BA + 0x01)

#define CS1262_ENTER_DEEPSLEEP_CMD 0x08C4
#define CS1262_EXIT_DEEPSLEEP_CMD  0x08C1
#define CS1262_CHIP_REST_CMD       0xC2

/****************************************** DEFINE ******************************************/
#define CS1262_SPI_NOCHECK_SINGLEWRITE        0x95
#define CS1262_SPI_CKSUNCHECK_SINGLEWRITE     0x96 // checksum for single write
#define CS1262_SPI_NOCHECK_CONTINUOUSWRITE    0x99
#define CS1262_SPI_CKSUNCHECK_CONTINUOUSWRITE 0x9A // checksum for continuous write

#define CS1262_SPI_NOCHECK_SINGLEREAD         0x65
#define CS1262_SPI_CKSUNCHECK_SINGLEREAD      0x66 // checksum for single read
#define CS1262_SPI_NOCHECK_CONTINUOUSREAD     0x69
#define CS1262_SPI_CKSUNCHECK_CONTINUOUSREAD  0x6A // checksum for continuous read

#define CS1262_SPI_ACK_ADDR  0xA3
#define CS1262_SPI_NACK_ADDR 0xA3
#define CS1262_SPI_ACK_DATA  0xA5
#define CS1262_SPI_NACK_DATA 0xAA

#define CS1262_SPI_DUMMY_DATA 0x00
/*********************************** function prototypes *************************************/
int32_t Cs1262ReadRegs(uint16_t regAddr, uint16_t *dataBuf, uint16_t dataLen);
int32_t Cs1262WriteReg(uint16_t regAddr, uint16_t data);
int32_t Cs1262WriteRegs(uint16_t regAddr, uint16_t *dataBuf, uint16_t dataLen);
int32_t Cs1262WriteRegbit(uint16_t regAddr, uint16_t setbit, Cs1262BitStatus bitval);
int32_t Cs1262WriteData(uint8_t *data, uint16_t dataLen);
int32_t Cs1262WriteGroup(Cs1262RegGroup *regGroup, uint16_t groupLen);
int32_t Cs1262ReadFifoReg(Cs1262FifoVal *fifoBuf, uint16_t fifoLen);
int32_t Cs1262InitSpi(struct SensorBusCfg *busCfg);
void Cs1262ReleaseSpi(struct SensorBusCfg *busCfg);
#endif /* CS1262_SPI_H */
