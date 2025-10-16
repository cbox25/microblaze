#include "FreeRTOS.h"
#include "task.h"
#include "flash.h"
#include "cmd.h"
#include <stdio.h>

#ifdef MODULE_SPI_FLASH
#include "xspi.h"

/*Define the bytes for write enable and write disable.*/
static uint8_t WriteEnableByte = WB_WRITE_ENABLE_BYTE;
static uint8_t WriteDisableByte = WB_WRITE_DISABLE_BYTE;
XSpi g_xspiDev[SPI_DEV_NUM_MAX];

/*SPI Module Initialization*/
__attribute__((optimize("O0"))) /* must not be deleted*/
void InitXspi(int8_t spiId)
{
    uint8_t vid[32] = { 0 };

    XSpi_Initialize(&g_xspiDev[spiId], spiId);
    XSpi_SetOptions(&g_xspiDev[spiId], XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
    XSpi_Start(&g_xspiDev[spiId]);

    memset(vid, 0x00, sizeof(vid));
    NorFlashReadVid(spiId, vid);
}

/*Flash write buffer function*/
/**prefix  A pointer to the prefix data*/
/*prefixCnt  Indicates the number of bytes in the prefix*/
/**tbuff  A pointer to the data to be sent*/
/**rbuff  A pointer to the data to be received*/
/*count  Indicates the number of bytes to send or receive*/
/*buff  The number of bytes to skip before filling rbuff*/
__attribute__((optimize("O0"))) /* must not be deleted*/
int ReadWriteNorFlash(int8_t spiId, uint8_t *prefix, int prefixCnt, uint8_t *tbuff, uint8_t *rbuff, int count, int skip)
{
    /* Obtain the base address of the SPI flash*/
    uint32_t base = g_xspiDev[spiId].BaseAddr;
    /*Define control and state variables*/
    uint32_t control;
    uint32_t status;

    /*Reset the send buffer FIFO and stop transmission.*/
    control = XSpi_ReadReg(base, XSP_CR_OFFSET);
    XSpi_WriteReg(base, XSP_CR_OFFSET, control | SPI_MASTER_INHIBIT_RESET);

    /*Fill FIFO*/
    int i;
    /*Write prefix data to the FIFO*/
    for (i = 0;i < prefixCnt;i++) {
        XSpi_WriteReg(base, XSP_DTR_OFFSET, *prefix++);
    }

    /*Write the data from tbuff to the FIFO.*/
    for (i = 0;i < count;i++) {
        XSpi_WriteReg(base, XSP_DTR_OFFSET, *tbuff++);
    }

    /*Select from device*/
    XSpi_WriteReg(base, XSP_SSR_OFFSET, SPI_FLASH_SELECT);

    /*Begin transmission. Do not be interrupted.*/
    taskENTER_CRITICAL();
    XSpi_WriteReg(base, XSP_CR_OFFSET, control & ~XSP_CR_TRANS_INHIBIT_MASK);
    /*Waiting for the transfer to complete*/
    do {
        status = XSpi_ReadReg(base, XSP_IISR_OFFSET);
    } while ((status & XSP_INTR_TX_EMPTY_MASK) == 0);
    taskEXIT_CRITICAL();

    /*Clear the transmission complete flag*/
    XSpi_WriteReg(base, XSP_IISR_OFFSET, XSpi_ReadReg(base, XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK);
    /*Stop transmission*/
    XSpi_WriteReg(base, XSP_CR_OFFSET, XSpi_ReadReg(base, XSP_CR_OFFSET) | XSP_CR_TRANS_INHIBIT_MASK);
    /*Deselect device*/
    XSpi_WriteReg(base, XSP_SSR_OFFSET, SPI_FLASH_UNSELECT);

    int j = 0;
    uint8_t c;

    taskENTER_CRITICAL();
    /*Reading data from the FIFO if rbuff exists*/
    for (i = 0; rbuff && (i < prefixCnt + count); ) {
        status = XSpi_ReadReg(base, XSP_SR_OFFSET);
        /*If the receiving register is not empty*/
        if ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
            c = (uint8_t)XSpi_ReadReg(base, XSP_DRR_OFFSET);
            /*Do not skip reading bytes that need to be read.*/
            if (i >= skip) {
                rbuff[j++] = c;
            }
            i++;
        }
    }
    taskEXIT_CRITICAL();

    /*Reset FIFO and disable transmission*/
    XSpi_WriteReg(base, XSP_CR_OFFSET, control | SPI_MASTER_INHIBIT_RESET);
    return 0;
}

/*Read status*/
uint8_t NorFlashReadStatus(int8_t spiId)
{
    uint8_t wbf[2] = { WB_READ_STATUS_REG1, DUMMYBYTE };
    ReadWriteNorFlash(spiId, wbf, 2, NULL, wbf, 0, 1);
    return wbf[0];
}

/*Write data*/
__attribute__((optimize("O0"))) /* must not be deleted*/
int NorFlashWriteBuf(int8_t spiId, uint32_t addr, int len, uint8_t *buff)
{
    int ret;
    uint8_t prefix[5];
    /*Write Enable*/
    WriteEnable(spiId);
    prefix[0] = W25_PAGE_PROGRAM;
    prefix[1] = (uint8_t)(addr >> 24);
    prefix[2] = (uint8_t)(addr >> 16);
    prefix[3] = (uint8_t)(addr >> 8);
    prefix[4] = (uint8_t)addr;
    ret = ReadWriteNorFlash(spiId, prefix, 5, buff, NULL, len, 0);
    /*Wait until the flash is free.*/
    NorFlashWaitNoBusy(spiId);
    /*Write Disenable*/
    WriteDisable(spiId);

    return ret;
}

/*Read data from a specified address*/
int NorFlashReadBuf(int8_t spiId, uint32_t addr, int len, uint8_t *buff)
{
    uint8_t prefix[5];
    prefix[0] = W25_READ_DATE;
    prefix[1] = (uint8_t)(addr >> 24);
    prefix[2] = (uint8_t)(addr >> 16);
    prefix[3] = (uint8_t)(addr >> 8);
    prefix[4] = (uint8_t)addr;
    ReadWriteNorFlash(spiId, prefix, 5, buff, buff, len, 5);
    return 0;
}

/*Erase function*/
__attribute__((optimize("O0"))) /* must not be deleted*/
int NorFlashErase(int8_t spiId, uint8_t cmd, uint32_t addr)
{
    int ret = 0;

    WriteEnable(spiId);
    uint8_t wbf[5] = {cmd, (addr >> 24) & 0xff,(addr >> 16) & 0xff, (addr >> 8) & 0xff, 0x00 };
    ret = ReadWriteNorFlash(spiId, wbf, 5, NULL, NULL, 0, 0);

    NorFlashWaitNoBusy(spiId);
    WriteDisable(spiId);

    return ret;
}

/*Read flash id*/
void NorFlashReadVid(int8_t spiId, uint8_t *vid)
{
    uint8_t wbf[32] = { WB_READ_ID, 0x00, 0x00, 0x00, 0x00, 0x00 };
    ReadWriteNorFlash(spiId, wbf, 16, NULL, vid, 0, 0);
}

__attribute__((optimize("O0"))) /* must not be deleted*/
void NorFlashRead(int8_t spiId, uint32_t addr, int len, uint8_t *buff)
{
    int rd = 0;
    for (int i = 0;i < len;i += rd) {
        rd = len - i > NOR_FLASH_RW_MAX_SIZE ? NOR_FLASH_RW_MAX_SIZE : len - i;
        NorFlashReadBuf(spiId, addr + i, rd, (uint8_t *)buff + i);
    }
}

void NorFlashEraseEntire(int8_t spiId)
{
    int ret = 0;

    WriteEnable(spiId);

    uint8_t wbf[1] = { WB_CHIP_ERASE };
    ret = ReadWriteNorFlash(spiId, wbf, 1, NULL, NULL, 0, 0);

    NorFlashWaitNoBusy(spiId);

    WriteDisable(spiId);
}

__attribute__((optimize("O0"))) /* must not be deleted*/
void NorFlashEraseBySector(int8_t spiId, uint32_t addr, uint32_t len)
{
    if (len == 0)
        return;

    uint32_t sectorNum = (len + SECTOR_SIZE - 1) / SECTOR_SIZE;
    /* align address to SECTOR_SIZE */
    uint32_t flashAddress = addr & (~(SECTOR_SIZE - 1));

    for (int i = 0; i < sectorNum; i++) {
        NORFLASH_ERASE_SECTOR(spiId, flashAddress + i * SECTOR_SIZE);
    }
}

__attribute__((optimize("O0"))) /* must not be deleted*/
int NorFlashWriteData(int8_t spiId, uint32_t flashAddr, uint8_t *buf, uint32_t len)
{
    if (spiId > SPI_DEV_NUM_MAX||!buf || len == 0)
    	return -1;
    uint32_t pageAddr = 0;
    uint32_t remaining= 0;
    uint32_t writeSize= 0;
    /* Keep the original static buffer to reduce stack usage (consistent with the original implementation) */
    uint8_t writeBuf[NOR_FLASH_RW_MAX_SIZE];
    uint8_t readBuf[NOR_FLASH_RW_MAX_SIZE];
    static uint8_t csvBlockBuf[CSV_BLOCK_SIZE];
    int writeLen = 0;
    int attempt = 0;
    int cmpResult = 0;
    uint32_t blockStartAddr = flashAddr & (~(CSV_BLOCK_SIZE - 1));
    if(blockStartAddr == flashAddr){
    	memset(csvBlockBuf, 0x00, CSV_BLOCK_SIZE);
    }
    memcpy(csvBlockBuf + (flashAddr - blockStartAddr), buf, len);
    /* Write in blocks, each write has a maximum of NOR_FLASH_RW_MAX_SIZE bytes */
    for (int offset = 0; offset < (int)len; offset += NOR_FLASH_RW_MAX_SIZE)
    {
        attempt = 0;
        cmpResult = 0;
        /* Clear the buffer and prepare the data block that needs to be written */
        memset(writeBuf, 0x00,sizeof(writeBuf));
        memset(readBuf, 0x00, sizeof(readBuf));
        writeLen = ((offset + NOR_FLASH_RW_MAX_SIZE) <= len) ? NOR_FLASH_RW_MAX_SIZE : (len - offset);
        memcpy(writeBuf, buf + offset, writeLen);
        /* Retry up to 5 times until the read-back data is consistent with the written data*/
        do{
            if (attempt == 0){
                /*First write the data block at the current address */
                NorFlashWriteBuf(SPI_DEV_ID_CODE, flashAddr, writeLen, writeBuf);
                DelayMs(10);

            }
            /* Read back the area just written and compare */
            NorFlashReadBuf(spiId, flashAddr, writeLen, readBuf);
            cmpResult = memcmp(writeBuf, readBuf, writeLen);
            if (cmpResult){
                LOG_DEBUG("flash rewrite\r\n");

                /*Calculate the sector (blockStartAddr is aligned to CSV_BLOCK_SIZE) and erase */

                NORFLASH_ERASE_4K_SECTOR(spiId, blockStartAddr);
                DelayMs(5);

                /* Merge the current write block into g_csvBlockBuf (keep the original behavior) */
                memcpy(csvBlockBuf + (flashAddr - blockStartAddr), writeBuf, writeLen);

                /* Calculate the number of pages to be written (with NOR_FLASH_RW_MAX_SIZE as the page size)*/
                uint32_t regionLen = (flashAddr - blockStartAddr) + writeLen;
                int totalPages = (regionLen + NOR_FLASH_RW_MAX_SIZE - 1) / NOR_FLASH_RW_MAX_SIZE;

                /* Write back the entire region in pages (except for the last page which may be a partial page) */
                for (int page = 0; page < totalPages; page++){
                     pageAddr = blockStartAddr + page * NOR_FLASH_RW_MAX_SIZE;
                     remaining = regionLen - page * NOR_FLASH_RW_MAX_SIZE;
                     writeSize = (remaining >= (uint32_t)NOR_FLASH_RW_MAX_SIZE) ? NOR_FLASH_RW_MAX_SIZE : remaining;
                     NorFlashWriteBuf(spiId, pageAddr, writeSize,
                                     csvBlockBuf + page * NOR_FLASH_RW_MAX_SIZE);
                   DelayMs(5);
                }
            }
            attempt++;
        } while (cmpResult != 0 && attempt < 5);
        /*If it still fails or there is a retry, print a warning (maintain the original behavior) */
        if (cmpResult != 0 || attempt > 1){
            LOG_DEBUG("warning wr flash, r: %d, c: %d, a: 0x%08x, off: %u, wr_b: %u\r\n",
                       cmpResult, attempt, flashAddr, offset, writeLen);
        }
        flashAddr += writeLen;
    }
}

__attribute__((optimize("O0"))) /* must not be deleted*/
void NorFlashWriteDataWithoutReadback(int8_t spiId, uint32_t flashAddr, uint8_t *buf, uint32_t len)
{
    uint32_t writtenLen = 0, writeByte = 0, currentAddr = 0, blockStartAddr = 0, i = 0;

    if (spiId >= SPI_DEV_NUM_MAX || buf == NULL || len == 0) {
        return;
    }

    currentAddr = flashAddr;
    for (writtenLen = 0; writtenLen < len; writtenLen += NOR_FLASH_RW_MAX_SIZE) {
        writeByte = (writtenLen + NOR_FLASH_RW_MAX_SIZE <= len) ? NOR_FLASH_RW_MAX_SIZE : len - writtenLen;

        NorFlashWriteBuf(spiId, currentAddr, writeByte, buf + writtenLen);
        DelayMs(3);
        currentAddr += writeByte;
    }
}

#endif /* MODULE_SPI_FLASH */
