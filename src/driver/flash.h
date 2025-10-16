#ifndef  FLASH_H
#define  FLASH_H

#include "common.h"

#ifdef MODULE_SPI_FLASH

/*
    flash memory space partitioning
    1. 0 ~ 11MB, Firmware of golden
    2. 11 ~ 28MB, Firmware of update
    3. 28 ~ 29MB, gencp xml
    4. 29 ~ 30MB, save camera arguments
    5. 30 ~ 31MB, FFC arguments
*/
/* FLASH CODE */
#define FLASH_SPACE_FIRMWARE_GOLDEN_START_ADDR   0

#ifdef FPGA_FUDAN_MICRO
#define FLASH_SPACE_FIRMWARE_UPDATE_START_ADDR   (0x00000000) /* 0MB */
#else
#define FLASH_SPACE_FIRMWARE_UPDATE_START_ADDR   (0x00B00000) /* 11MB */
#endif

/* GENCP XML */
#define FLASH_SPACE_GENCP_XML_INFO_START_ADDR    (0x01C00000) /* 28MB, length is 128B used to storage the xml info including 4B xml len and 20B sha1-hash */
#define FLASH_SPACE_GENCP_XML_DATA_START_ADDR    (0x01C00080) /* 28MB + 128B */
/* CAMERA REG ARG */
#define FLASH_SPACE_REG_ARG_START_ADDR0          (0x01D00000) /* 29MB */
#define FLASH_SPACE_REG_ARG_START_ADDR1          (0x01D02000) /* 29MB + 8KB */
#define FLASH_SPACE_REG_ARG_START_ADDR2          (0x01D04000) /* 29MB + 16KB */
#define FLASH_SPACE_REG_ARG_MODE_ADDR            (0x01D06000) /* 29MB + 24KB */
#define FLASH_SPACE_CSV_START_ADDR               (0x01D08000) /* 29MB + 32KB, space: 8KB */
/* ISP arg */
/* FFC arg */
#define FLASH_SPACE_FFC_DARK_FIELD_START_ADDR    (0x01E00000) /* 30MB, space: 8KB */
#define FLASH_SPACE_FFC_PRNU_HIGH_START_ADDR     (0x01E02000) /* 30MB + 8KB, space: 8KB */
#define FLASH_SPACE_FFC_PRNU_LOW_START_ADDR      (0x01E04000) /* 30MB + 16KB, space: 8KB */
/* DPC arg */
#define FLASH_SPACE_DPC_START_ADDR               (0x01E06000) /* 30MB + 24KB, space: 8KB */


#define NOR_FLASH_RW_MAX_SIZE            128
#define NOR_FLASH_ERASE_BLOCK_SIZE_4K    4096

#define W25_READ_DATE                    0x13 /*Read command*/
#define W25_PAGE_PROGRAM                 0x12 /*Page Programming*/
#define W25_WRITE_STATUS_REG             0x01 /*Write status register*/
#define WB_WRITE_ENABLE_BYTE             0x06 /*Define the write enable command*/
#define WB_WRITE_DISABLE_BYTE            0x04 /*Define the write-disabled byte command*/
#define WB_READ_STATUS_REG1              0x05 /*Define Read Status Register 1 Command*/
// #define WB_READ_STATUS_REG2           0x07 /*Define Read Status Register 2 Command*/
// #define WB_SECTOR_ERASE 0xDC          /*20h A23-A16 A15-A8 A7-A0*/
#define WB_32K_SUBSECTOR_ERASE           0x52
#define WB_4K_SUBSECTOR_ERASE            0x21 /* erase 4K subsector */
#define WB_SECTOR_ERASE                  0xDC /* 64KB */ /*Command to define erased sectors*/
#define WB_CHIP_ERASE                    0x60 /*1BYTE*/ /*Define commands for erasing chips*/
#define WB_READ_ID                       0x9f /*90H DUMMY DUMMY 00h (MF7-MF0) (ID7-ID0)*/ /*Define the command to read the Flash ID*/
#define SECTOR_SIZE                      (64*1024) /*定义扇区大小*/

/*Virtual Byte*/
#define DUMMYBYTE            0xFF
/*The mask indicating the non-busy state in the status register of the Flash device*/
#define FLASH_SR_IS_READY_MASK    0x01 /* Ready mask */
/*Define the signal on the SPI bus for selecting the flash device.*/
#define SPI_FLASH_SELECT   0x00
#define SPI_FLASH_UNSELECT 0xFFFFFFFF

#define XSP_SR_RX_NOT_EMPTY_MASK 0x01

/*Macro for Defining Erased Sectors*/
#define NORFLASH_ERASE_SECTOR(id, addr)       NorFlashErase(id, WB_SECTOR_ERASE, addr)
#define NORFLASH_ERASE_4K_SECTOR(id, addr)    NorFlashErase(id, WB_4K_SUBSECTOR_ERASE, addr)
#define ERASE_32K_SECTOR(id, addr)            NorFlashErase(id, WB_32K_SUBSECTOR_ERASE, addr)

/*Define wait until flash is not busy*/
#define NorFlashWaitNoBusy(id) while(NorFlashReadStatus(id) & FLASH_SR_IS_READY_MASK)

#define SPI_FIFO_RESET              (XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK)
#define SPI_MASTER_INHIBIT_RESET    (SPI_FIFO_RESET | XSP_CR_TRANS_INHIBIT_MASK)

/*Some macro definitions simplify operations.*/
#define SELECT_FLASH(id)          XSpi_SetSlaveSelect(&g_xspiDev[id], SPI_FLASH_SELECT)
#define UNSELECT_FLASH(id)        XSpi_SetSlaveSelect(&g_xspiDev[id], 0)
#define CS_SELECT(id)             XSpi_SetSlaveSelectReg(&g_xspiDev[id], 1)
#define CS_UNSELECT(id)           XSpi_SetSlaveSelectReg(&g_xspiDev[id], 0)
#define WriteEnable(id)           ReadWriteNorFlash(id, &WriteEnableByte, 1, NULL, NULL, 0, 0)
#define WriteDisable(id)          ReadWriteNorFlash(id, &WriteDisableByte, 1, NULL, NULL, 0, 0)

extern XSpi g_xspiDev[SPI_DEV_NUM_MAX];

/*Functions for sending commands and data*/
int ReadWriteNorFlash(int8_t spiId, uint8_t *prefix, int prefixCnt, uint8_t *tbuff, uint8_t *rbuff, int count, int skip);
/*Flash write buffer function*/
int NorFlashWriteBuf(int8_t spiId, uint32_t addr, int len, uint8_t *buff);
/*Flash read buffer function*/
int NorFlashReadBuf(int8_t spiId, uint32_t addr, int len, uint8_t *buff);
/*Chip Erasure Function*/
void NorFlashEraseEntire(int8_t spiId);
/*Function to read the VID of a flash drive*/
void NorFlashReadVid(int8_t spiId, uint8_t *vid);
/*Flash erase function*/
int NorFlashErase(int8_t spiId, uint8_t cmd, uint32_t addr);
void NorFlashEraseBySector(int8_t spiId, uint32_t addr, uint32_t len);
/*Flash read function*/
void NorFlashRead(int8_t spiId, uint32_t addr, int len, uint8_t *buff);
/*SPI Module Initialization*/
void InitXspi(int8_t spiId);
uint8_t NorFlashReadStatus(int8_t spiId);
int NorFlashWriteData(int8_t spiId, uint32_t flashAddr, uint8_t *buf, uint32_t len);
void NorFlashWriteDataWithoutReadback(int8_t spiId, uint32_t flashAddr, uint8_t *buf, uint32_t len);

#endif /* MODULE_SPI_FLASH */
#endif /* FLASH_H */
