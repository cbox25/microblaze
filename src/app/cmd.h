/*
 * cmd.h
 *
 *  Created on: 2025.2.22.
 *      Author: Administrator
 */

#ifndef CMD_H
#define CMD_H

#include "uart.h"
#include "packet_sr.h"
#include "common.h"

#ifdef MODULE_UART
enum {
    CMD_TYPE_SAVE_ARG                = 0x01,
    CMD_TYPE_SET_UART_BAUDRATE       = 0x02,
    CMD_TYPE_FFC_CALIBRATE           = 0x03,
	CMD_TYPE_SAVE_CSV                = 0x04,
	CMD_TYPE_SEND_CSV                = 0x05,
	CMD_TYPE_SEND_VERSION			 = 0x06,
	CMD_TYPE_SAVE_ARG_WITHOUT_DATA   = 0x07,
    CMD_TYPE_NUM_MAX                 = 0x08,
};

#ifdef ISP_FFC
enum {
    CMD_ACK_TYPE_FFC_DARK_DATA = 1,
    CMD_ACK_TYPE_FFC_PRNU_HIGH_DATA,
    CMD_ACK_TYPE_FFC_PRNU_LOW_DATA
};

#define FFC_CALIB_DARK                  0x01
#define FFC_CALIB_BRIGHT                0x02

#define FFC_ENABLE_ADDR                 0xB84
#define FFC_FPN_CALIB_EN_ADDR           0xB94
#define FFC_BRIGHT_CALIB_EN_ADDR    	0xB98
#define FFC_PRNU_CFG_EN_ADDR			0xB9C
#define FFC_FPN_CALIB_RDY_ADDR          0xBA0
#define FFC_PRNU_DONE_ADDR              0xBA4
#define FFC_DARK_DATA_RD_ADDR           0xBA8
#define FFC_DARK_DATA_WR_ADDR           0xBAC
#define FFC_PRNU_HIGH_DATA_RD_ADDR      0xBB0
#define FFC_PRNU_LOW_DATA_RD_ADDR       0xBB4
#define FFC_PRNU_HIGH_DATA_WR_ADDR      0xBB8
#define FFC_PRNU_LOW_DATA_WR_ADDR       0xBBC

#define FFC_NOR_FLASH_RW_LEN            32
#define FFC_DATA_LEN                    1024
#endif

/* save args */
#ifdef LINESCAN
#define INTER_EXTE_FLAG_ADDR           0xA40
#define WAVE_DLY_ADDR                  0xA44
#define JITTER_TIME_ADDR               0xA48
#define PULSE_WIDTH_ADDR               0xA4C
#define EXERT_FRAME_FREQ_ADDR          0xA50
#define INSERT_FRAME_FREQ_ADDR         0xA54
#define INSER_EXP_TIME_SET_ADDR        0xA58
#define EXT_TRIG_MODE_ADDR             0xA5C
#define SENSOR_MODE_CHOOSE_ADDR        0xA80
#define HG_COARSE_GAIN_ADDR            0xA84
#define HG_FINE_GAIN_ADDR              0xA88
#define LG_COARSE_GAIN_ADDR            0xA8C
#define LG_FINE_GAIN_ADDR              0xA90
#define CMOS_COARSE_GAIN_ADDR          0xA94
#define CMOS_FINE_GAIN_ADDR            0xA98
#define HDR_MODE_ADDR                  0xAC0
#define HDR_THR_ADDR                   0xAC4
#define HDR_K_ADDR                     0xAC8
#define HDR_B_ADDR                     0xACC
#define SET_PGA_EN_ADDR                0xB00
#define SET_PGA_ADDR                   0xB04
#define DPC_EN_ADDR                    0xB40
#define DPC_REMARK_ADDR                0xB44
#define DPC_MARK_WRDATA_ADDR           0xB48
#define DPC_MARK_NUM_ADDR              0xB4C
#define FFC_HDR_MODE                   0xB80
#define FFC_EN_ADDR                    0xB84
#define FFC_RECALIB_ADDR               0xB88
#define FFC_TARGET_ADDR                0xB8C
#define FFC_MODE_ADDR                  0xB90
#define FPN_CALIB_EN_ADDR              0xB94
#define BRIGHT_CALIB_EN_ADDR           0xB98
#define PRNU_CFG_EN_ADDR               0xB9C
#define FPN_CALIB_RDY_ADDR             0xBA0
#define ACQUISITION_START_EN_ADDR      0xBC0
#define TEMPERATURE_ADDR               0xBC4
#define TEST_IMAGE_ADDR                0xBC8
#define X_MIRROR_ADDR                  0xBCC
#define IMAGE_HIGHT_ADDR               0xC00
#define IMAGE_WIDTH_ADDR               0xC04
#define PIXEL_FORMAT_ADDR              0xC08
#define PAYLOAD_SIZE_ADDR              0xC40
#define ACQ_MODE_ADDR                  0xBD0
#define FX3_MODE0_ADDR                 0xBD4
#define TEMPERATURE_BIAS_ADDR		   0xBD8
#define STD_BLC_EN_ADDR			       0xCC0
#define STD_BLC_WRITE_DATA_ADDR	       0xCC4
#define HDR_HG_BLC_EN_ADDR	           0xCC8
#define HDR_HG_BLC_WRITE_DATA_ADDR     0xCCC
#define HDR_LG_BLC_EN_ADDR	           0xCD0
#define HDR_LG_BLC_WRITE_DATA_ADDR     0xCD4
#define BLC_HG_N                       0xCD8
#define BLC_HG_P                       0xCDC
#define BLC_LG_N                       0xCE0
#define BLC_LG_P                       0xCE4
#define BIT_SHIFT_EN_ADDR	           0xD00
#define BIT_SHIFT_CHOOSE_ADDR	       0xD04

#define REG_LIST_TOTAL_NUM     36
#elif defined(QIYI3412)

#define DIGI_GAIN		0x988
#define PGA_GAIN		0x984
#define INSERT_FRAME	0x9EC
#define X_MIRROR		0x998
#define Y_MIRROR		0x99C
#define SET_EXP_TIME	0x9C4
#define RES_SEL			0xA08
#define SENSOR_X_START	0xA0C
#define SENSOR_Y_START	0xA10
#define IMAGE_CAPTURE	0xA00


#define REG_LIST_TOTAL_NUM     10
#else
#define REG_LIST_TOTAL_NUM     0
#endif

extern Reg g_saveRegs[];

#define ARG_MODE_MAX	3
#define VERSION_LENGTH  30
#define DPC_NUM_MAX     512
#define CSV_BLOCK_SIZE  4096

typedef struct {
	uint32_t argMode;
	int deviceTempBias[ARG_MODE_MAX];
} GlobalParams;

typedef struct {
	uint8_t  cmdType;
	uint8_t  csvStatus;
	uint16_t packNum;
} MessageReplyPackWithPackNum;

typedef struct {
	uint8_t cmdType;
	uint8_t cmdStatus;
} MessageReplyPack;

typedef struct {
	uint8_t cmdType;
	uint8_t cmdStatus;
	uint8_t cmdVersion[VERSION_LENGTH];
} SendVersionPack;

extern UartQueue g_queueUart[QUEUE_UART_NUM];
extern GlobalParams g_globalParams;

void HandleSaveArgs(SerialData *receivedData);
void HandleSetUartBaudrate(SerialData *receivedData);
void HandleSendCsv(SerialData *recvData);
void HandleSaveCsv(SerialData *recvData);
void HandleFfcCalibrate(SerialData *recvData);
void HandleSendVersion(SerialData *recvData);
void InitFfc(void);
void FfcCalibrateDark(void);
void FfcCalibrateBright(void);
void InitDpc(void);
void SaveArgsToFlash(uint32_t argMode);
void LoadArgsFromFlash(uint32_t argMode);
void InitWriteCameraRegListToFlash(void);
void HandleSaveArgsWithoutData(SerialData *receivedData);
void CmdTask(void *unused);
#endif /* MODULE_UART */

#endif /* CMD_H */
