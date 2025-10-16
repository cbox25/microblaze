/*
 * cmd.c
 *
 *  Created on: 2025年2月22日
 *      Author: Administrator
 */

#include "uart.h"
#include "cmd.h"
#include "packet_sr.h"
#include "common.h"

#ifdef MODULE_UART
typedef void (*CmdCb)(SerialData *recvData);

CmdCb cmdFunc[CMD_TYPE_NUM_MAX] = {
        NULL,
        HandleSaveArgs,
        HandleSetUartBaudrate,
        HandleFfcCalibrate,
		HandleSaveCsv,
		HandleSendCsv,
		HandleSendVersion,
		HandleSaveArgsWithoutData
};
#endif /* MODULE_UART */

GlobalParams g_globalParams;

static uint32_t g_regFlashAddr[] = {
    FLASH_SPACE_REG_ARG_START_ADDR0,
    FLASH_SPACE_REG_ARG_START_ADDR1,
    FLASH_SPACE_REG_ARG_START_ADDR2
};

#if defined(LINESCAN)
/* save args */
Reg g_saveRegs[REG_LIST_TOTAL_NUM] = {
        {AXI_BASE_ADDR + INTER_EXTE_FLAG_ADDR       ,0,10},
        {AXI_BASE_ADDR + WAVE_DLY_ADDR              ,0,10},
        {AXI_BASE_ADDR + JITTER_TIME_ADDR           ,0,10},
        {AXI_BASE_ADDR + PULSE_WIDTH_ADDR           ,0,10},
        {AXI_BASE_ADDR + EXERT_FRAME_FREQ_ADDR      ,0,10},
        {AXI_BASE_ADDR + INSERT_FRAME_FREQ_ADDR     ,0,10},
        {AXI_BASE_ADDR + INSER_EXP_TIME_SET_ADDR    ,0,10},
        {AXI_BASE_ADDR + SENSOR_MODE_CHOOSE_ADDR    ,0,10},
        {AXI_BASE_ADDR + HG_COARSE_GAIN_ADDR        ,0,10},
        {AXI_BASE_ADDR + HG_FINE_GAIN_ADDR          ,0,10},
        {AXI_BASE_ADDR + LG_COARSE_GAIN_ADDR        ,0,10},
        {AXI_BASE_ADDR + LG_FINE_GAIN_ADDR          ,0,10},
        {AXI_BASE_ADDR + CMOS_COARSE_GAIN_ADDR      ,0,10},
        {AXI_BASE_ADDR + CMOS_FINE_GAIN_ADDR        ,0,10},
        {AXI_BASE_ADDR + HDR_MODE_ADDR              ,0,10},
        {AXI_BASE_ADDR + HDR_THR_ADDR               ,0,10},
        {AXI_BASE_ADDR + HDR_K_ADDR                 ,0,10},
        {AXI_BASE_ADDR + HDR_B_ADDR                 ,0,10},
        {AXI_BASE_ADDR + SET_PGA_EN_ADDR            ,0,10},
        {AXI_BASE_ADDR + SET_PGA_ADDR               ,0,10},
        {AXI_BASE_ADDR + DPC_EN_ADDR                ,0,10},
        {AXI_BASE_ADDR + DPC_REMARK_ADDR            ,0,10},
        {AXI_BASE_ADDR + DPC_MARK_NUM_ADDR          ,0,10},
        {AXI_BASE_ADDR + FFC_HDR_MODE               ,0,10},
        {AXI_BASE_ADDR + FFC_EN_ADDR                ,0,10},
        {AXI_BASE_ADDR + ACQUISITION_START_EN_ADDR  ,0,10},
        {AXI_BASE_ADDR + TEMPERATURE_ADDR           ,0,10},
        {AXI_BASE_ADDR + TEST_IMAGE_ADDR            ,0,10},
        {AXI_BASE_ADDR + X_MIRROR_ADDR              ,0,10},
        {AXI_BASE_ADDR + IMAGE_HIGHT_ADDR           ,0,10},
        {AXI_BASE_ADDR + IMAGE_WIDTH_ADDR           ,0,10},
        {AXI_BASE_ADDR + PIXEL_FORMAT_ADDR          ,0,10},
        {AXI_BASE_ADDR + PAYLOAD_SIZE_ADDR          ,0,10},
        {AXI_BASE_ADDR + ACQ_MODE_ADDR              ,0,10},
        {AXI_BASE_ADDR + FX3_MODE0_ADDR             ,0,10},
        {AXI_BASE_ADDR + EXT_TRIG_MODE_ADDR         ,0,10}
};
#elif defined(QIYI3412)

Reg g_saveRegs[REG_LIST_TOTAL_NUM] = {
		{AXI_BASE_ADDR + DIGI_GAIN		,0,10},
		{AXI_BASE_ADDR + PGA_GAIN		,0,10},
		{AXI_BASE_ADDR + INSERT_FRAME	,0,10},
		{AXI_BASE_ADDR + X_MIRROR		,0,10},
		{AXI_BASE_ADDR + Y_MIRROR		,0,10},
		{AXI_BASE_ADDR + SET_EXP_TIME	,0,10},
		{AXI_BASE_ADDR + RES_SEL		,0,10},
		{AXI_BASE_ADDR + SENSOR_X_START	,0,10},
		{AXI_BASE_ADDR + SENSOR_Y_START	,0,10},
		{AXI_BASE_ADDR + IMAGE_CAPTURE	,0,10}
};
#else
Reg g_saveRegs[] = {};
#endif

#if defined(MODULE_SPI_FLASH) && defined(MODULE_UART)
void HandleSaveArgs(SerialData *receivedData)
{
    if (!receivedData)
        return;

    Pack *pkt = (Pack *)receivedData->data;
    int dataLen = (int)pkt->dataLen;
    int bufLen = 0, remain = 0, writeLen = 0;
    uint8_t *data = pkt->dataPayload;
    uint16_t crc = 0;
    uint32_t blockStartAddr = FLASH_SPACE_REG_ARG_START_ADDR0;

    int blkNum = (dataLen + NOR_FLASH_RW_MAX_SIZE - 1) / NOR_FLASH_RW_MAX_SIZE, i = 0;
    uint8_t dataBuf[NOR_FLASH_RW_MAX_SIZE];

    /* crc */
    memcpy(&crc, data + dataLen, 2);
    if (!CheckCrc16(receivedData->data, receivedData->size - 2, crc)) {
        LOG_DEBUG("check crc error\r\n");
        /* send error packet */
        return;
    }
    LOG_DEBUG("crc OK\r\n");

    /* write data to flash */
    dataLen -= 1; /* remove cmdType */
    data += 1;
    memset(dataBuf, 0x00, sizeof(dataBuf));
    memcpy(dataBuf, &dataLen, sizeof(dataLen));
    bufLen += sizeof(dataLen) + 4; /* write data len to first 8 bytes */

    LOG_DEBUG("start erase flash\r\n");
    /* erase block */
    NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_ARG, blockStartAddr);
    DelayMs(3);

    LOG_DEBUG("erase flash OK\r\n");

    /* write block */
    remain = dataLen;
    /* write first block with data length */
    writeLen = remain < (NOR_FLASH_RW_MAX_SIZE - 8) ? remain : (NOR_FLASH_RW_MAX_SIZE - 8);
    memcpy(dataBuf + bufLen, data, writeLen);

    NorFlashWriteBuf(SPI_DEV_ID_ARG, blockStartAddr, writeLen + bufLen, dataBuf);
    remain -= writeLen;
    data += writeLen;

    /* write other block */
    NorFlashWriteDataWithoutReadback(SPI_DEV_ID_ARG, blockStartAddr + NOR_FLASH_RW_MAX_SIZE, data, remain);
}

static inline void UpdateCameraRegList(void)
{
	for (int i = 0; i < sizeof(g_saveRegs) / sizeof(g_saveRegs[0]); i ++) {
		ReadReg(&g_saveRegs[i]);
		LOG_DEBUG("a: 0x%x, v: 0x%x\r\n", g_saveRegs[i].addr, g_saveRegs[i].value);
		DelayMs(2);
	}
}

void HandleSaveArgsWithoutData(SerialData *receivedData)
{
    if (!receivedData)
        return;

    Pack *pkt = (Pack *)receivedData->data;
    int dataLen = (int)pkt->dataLen, bufLen = 0, remain = 0, writeLen = 0;
    uint8_t *data = pkt->dataPayload;
    uint16_t crc = 0;
    memcpy(&crc, data + dataLen, 2);
    /* crc */
    if (!CheckCrc16(receivedData->data, receivedData->size - 2, crc)) {
        LOG_DEBUG("check crc error\r\n");
        /* send error packet */
        return;
    }
    LOG_DEBUG("crc OK\r\n");

	UpdateCameraRegList();
	SaveArgsToFlash(g_globalParams.argMode);
}

#ifdef ISP_FFC
void FfcCalibrateDark(void)
{
    uint32_t value = 0;
    uint32_t data[FFC_NOR_FLASH_RW_LEN];
    int i = 0, j = 0;
    uint32_t startFlag = 0;

    LOG_DEBUG("FFC dark start\r\n");

    WriteRegByAddr(AXI_BASE_ADDR + FFC_EN_ADDR, 0);

    DelayMs(10);

    WriteRegByAddr(AXI_BASE_ADDR + FFC_EN_ADDR, 1);
    DelayMs(10);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_PRNU_CFG_EN_ADDR, 1);
    DelayMs(10);

    WriteRegByAddr(AXI_BASE_ADDR + FFC_RECALIB_ADDR, 1);

    DelayMs(2000);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_RECALIB_ADDR, 0);

    WriteRegByAddr(AXI_BASE_ADDR + FFC_FPN_CALIB_EN_ADDR, 1); /* write fpn_calib_en as 1 */
    DelayMs(4000);

    /* read until fpn_calib_rdy is 1, read dark data and write to FLASH */
    do {
        value = ReadRegByAddr(AXI_BASE_ADDR + FFC_FPN_CALIB_RDY_ADDR);
    } while (value != 1);

    memset(data, 0x00, sizeof(data));

    /* erase 8KB block */
    NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_DARK_FIELD_START_ADDR);
    DelayMs(5);
    NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_DARK_FIELD_START_ADDR + 0x1000);
    DelayMs(50); /* wait FPGA data */
    startFlag = 0;
    for (i = 0; i < FFC_DATA_LEN / FFC_NOR_FLASH_RW_LEN; i ++) {
        j = 0;
    	while (j < FFC_NOR_FLASH_RW_LEN) {
            data[j] = ReadRegByAddr(AXI_BASE_ADDR + FFC_DARK_DATA_RD_ADDR);
            if (data[j] == 0 && startFlag == 0)
				continue;

			startFlag = 1;
			j ++;
        }

        NorFlashWriteBuf(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_DARK_FIELD_START_ADDR + i * sizeof(data),
                         sizeof(data), data);
        DelayMs(3);
    }
    DelayMs(4000);
    WriteRegByAddr(AXI_BASE_ADDR + FPN_CALIB_EN_ADDR,0);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_EN_ADDR,0);

    LOG_DEBUG("FFC_DARK OK\r\n");
}

void FfcCalibrateBright(void)
{
    uint32_t value = 0;
    uint32_t data[FFC_NOR_FLASH_RW_LEN];
    int i = 0, j = 0;
    uint8_t startFlag = 0;

    LOG_DEBUG("FFC bright start\r\n");

    WriteRegByAddr(AXI_BASE_ADDR + FFC_EN_ADDR,0);

    DelayMs(10);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_EN_ADDR,1);
    DelayMs(10);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_BRIGHT_CALIB_EN_ADDR, 1); /* write bright_calib_en as 1 */
    DelayMs(4000);

    /* Read until prnu_done is 1 */
    while (value != 1) {
        value = ReadRegByAddr(AXI_BASE_ADDR + FFC_PRNU_DONE_ADDR);
    }

    LOG_DEBUG("FFC_PRNU_DONE OK\r\n");

    /* erase 16KB block */
    for (i = 0; i < 4; i ++) {
        NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_PRNU_HIGH_START_ADDR + i * 0x1000);
        DelayMs(5);
    }

    /* Read 1024 PRNU high data and store it in the high FFC zone of the FLASH */
    memset(data, 0x00, sizeof(data));
    DelayMs(50); /* wait FPGA data */
    startFlag = 0;
    for (i = 0; i < FFC_DATA_LEN / FFC_NOR_FLASH_RW_LEN; i ++) {
    	j = 0;
        while (j < FFC_NOR_FLASH_RW_LEN) {
            data[j] = ReadRegByAddr(AXI_BASE_ADDR + FFC_PRNU_HIGH_DATA_RD_ADDR);
            DelayUs(2);
            if (data[j] == 0 && startFlag == 0)
				continue;
			startFlag = 1;
			j ++;
        }

        NorFlashWriteBuf(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_PRNU_HIGH_START_ADDR + i * sizeof(data),
                        sizeof(data), data);
    }

    LOG_DEBUG("write FFC_PRNU_HIGH OK\r\n");

    /* Read 1024 PRNU low data and store it in the low FFC zone of the FLASH */
    memset(data, 0x00, sizeof(data));
    DelayMs(50); /* wait FPGA data */
    startFlag = 0;
    for (i = 0; i < FFC_DATA_LEN / FFC_NOR_FLASH_RW_LEN; i ++) {
        j = 0;
    	while (j < FFC_NOR_FLASH_RW_LEN) {
            data[j] = ReadRegByAddr(AXI_BASE_ADDR + FFC_PRNU_LOW_DATA_RD_ADDR);
            DelayUs(2);

            if (data[j] == 0 && startFlag == 0)
				continue;

			startFlag = 1;
			j ++;
        }

        NorFlashWriteBuf(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_PRNU_LOW_START_ADDR + i * sizeof(data),
                        sizeof(data), data);
    }

    LOG_DEBUG("write FFC_PRNU_LOW OK\r\n");

	DelayMs(4000);
	WriteRegByAddr(AXI_BASE_ADDR + FFC_BRIGHT_CALIB_EN_ADDR,0);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_PRNU_CFG_EN_ADDR,0);
    LOG_DEBUG("end of ffc bright\r\n");
}

void InitFfc(void)
{
    uint32_t data[FFC_NOR_FLASH_RW_LEN];
    int i = 0, j = 0;
    uint32_t value = 0;
    static uint16_t high = 1, low = 0;
	static uint32_t testData = 0;

    LOG_DEBUG("init ffc start\r\n");
    /* write ffc_en as 1 */
    WriteRegByAddr(AXI_BASE_ADDR + FFC_ENABLE_ADDR, 1);
    DelayMs(1);
    WriteRegByAddr(AXI_BASE_ADDR + FFC_FPN_CALIB_EN_ADDR, 0); /* write fpn_calib_en as 0 */
	DelayMs(1);
	WriteRegByAddr(AXI_BASE_ADDR + FFC_BRIGHT_CALIB_EN_ADDR, 0); /* write bright_calib_en as 0 */
	DelayMs(1);
	WriteRegByAddr(AXI_BASE_ADDR + FFC_PRNU_CFG_EN_ADDR, 0); /* write prnu_cfg_en as 0 */
	DelayMs(1);

	LOG_DEBUG("read dark field\r\n");
    /* read dark field */
	DelayMs(50); /* wait FPGA data */
    for (i = 0; i < FFC_DATA_LEN / FFC_NOR_FLASH_RW_LEN; i ++) {
        NorFlashReadBuf(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_DARK_FIELD_START_ADDR+ i * sizeof(data),
        				sizeof(data), data);

        for (j = 0; j < FFC_NOR_FLASH_RW_LEN; j ++) {
            WriteRegByAddr(AXI_BASE_ADDR + FFC_DARK_DATA_WR_ADDR, data[j]);
            DelayUs(1);
        }
    }

	LOG_DEBUG("wait fpn calib ready...\r\n");
    while (value != 1) {
        value = ReadRegByAddr(AXI_BASE_ADDR + FFC_FPN_CALIB_RDY_ADDR);
    }

    LOG_DEBUG("read prnu high field\r\n");
    /* read prnu high */
    DelayMs(50); /* wait for FPGA */
    for (i = 0; i < FFC_DATA_LEN / FFC_NOR_FLASH_RW_LEN; i ++) {
        NorFlashReadBuf(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_PRNU_HIGH_START_ADDR+ i * sizeof(data),
        				sizeof(data), data);

        for (j = 0; j < FFC_NOR_FLASH_RW_LEN; j ++) {
            WriteRegByAddr(AXI_BASE_ADDR + FFC_PRNU_HIGH_DATA_WR_ADDR, data[j]);
            DelayUs(1);
        }
    }

    LOG_DEBUG("read prnu low field\r\n");
    /* read prnu low */
    DelayMs(50); /* wait for FPGA */
    for (i = 0; i < FFC_DATA_LEN / FFC_NOR_FLASH_RW_LEN; i ++) {
        NorFlashReadBuf(SPI_DEV_ID_CODE, FLASH_SPACE_FFC_PRNU_LOW_START_ADDR + i * sizeof(data),
        				sizeof(data), data);

        for (j = 0; j < FFC_NOR_FLASH_RW_LEN; j ++) {
            WriteRegByAddr(AXI_BASE_ADDR + FFC_PRNU_LOW_DATA_WR_ADDR, data[j]);
            DelayUs(1);
        }
    }

    LOG_DEBUG("wait prnu done...\r\n");
    while (value != 1) {
        value = ReadRegByAddr(AXI_BASE_ADDR + FFC_PRNU_DONE_ADDR);
    }

    /* set ffc_en as 0 */
    WriteRegByAddr(AXI_BASE_ADDR + FFC_ENABLE_ADDR, 0);
	DelayMs(1);
}

void HandleFfcCalibrate(SerialData *recvData)
{
    if (recvData == NULL)
        return;

    /* parse uart buf */
    Pack *pkt = (Pack *)recvData->data;
    int dataLen = (int)pkt->dataLen;
    uint8_t *data = pkt->dataPayload;
    uint16_t crc = (*(data + dataLen)) + ((*(data + dataLen + 1)) << 8);
    uint32_t calibType = 0;

    /* crc */
    if (!CheckCrc16(recvData->data, recvData->size - 2, crc)) {
        LOG_DEBUG("check crc error\r\n");
        /* send error packet */
        return;
    }

    if (data[1] == FFC_CALIB_DARK) {
        FfcCalibrateDark();
    } else if (data[1] == FFC_CALIB_BRIGHT) {
        FfcCalibrateBright();
    } else {
        return;
    }
}
#else
void FfcCalibrateDark(void){}
void FfcCalibrateBright(void){}
void InitFfc(void){}
void HandleFfcCalibrate(SerialData *recvData){}
#endif

#if defined(ISP_DPC)
void InitDpc(void)
{
    uint32_t startAddr = 0, dataLen = 0, i = 0, blkNum = 0, offset = 0;
    uint8_t buf[NOR_FLASH_RW_MAX_SIZE] = {0};

    startAddr = FLASH_SPACE_DPC_START_ADDR;
    memset(buf, 0x00, sizeof(buf));
    /* read len and data from flash. the first 4B is len, and the second 4B is reserved */
    NorFlashReadBuf(SPI_DEV_ID_CODE, startAddr, sizeof(buf), buf);
    DelayMs(2);
    dataLen = *(uint32_t *)buf;

    dataLen = dataLen <= DPC_NUM_MAX ?  dataLen : DPC_NUM_MAX;

    /* write len to FPGA */
    WriteRegByAddr(AXI_BASE_ADDR + DPC_MARK_NUM_ADDR, dataLen);

    /* write data to FPGA */
    /* write first buf */
    offset = 8;
    blkNum = 1;
    for (i = 0; i < dataLen; i ++) {
        WriteRegByAddr(AXI_BASE_ADDR + DPC_MARK_WRDATA_ADDR, *(uint32_t *)(buf + offset));
        offset += sizeof(uint32_t);
        if (offset == NOR_FLASH_RW_MAX_SIZE) {
            NorFlashReadBuf(SPI_DEV_ID_CODE, startAddr + blkNum * NOR_FLASH_RW_MAX_SIZE, sizeof(buf), buf);
            DelayMs(2);
            offset = 0;
            blkNum ++;
        }
    }
}
#else
void InitDpc(void){}
#endif

void SaveArgsToFlash(uint32_t argMode)
{
    uint32_t flashArgAddr = 0;

    UpdateCameraRegList();

    flashArgAddr = g_regFlashAddr[argMode];

    /* erase flash 8K */
    NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_ARG, flashArgAddr);
    DelayMs(3);
    NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_ARG, flashArgAddr + 0x1000);
    DelayMs(3);
    NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_ARG, FLASH_SPACE_REG_ARG_MODE_ADDR); /* erase arg mode flash space */
    DelayMs(3);

    /* write data to flash */
    NorFlashWriteDataWithoutReadback(SPI_DEV_ID_CODE, flashArgAddr, (uint8_t *)g_saveRegs, sizeof(g_saveRegs));

    /* write arg mode to flash */
    NorFlashWriteBuf(SPI_DEV_ID_CODE, FLASH_SPACE_REG_ARG_MODE_ADDR, sizeof(g_globalParams), &g_globalParams);
}

#if defined(QIYI3412)
void LoadArgsFromFlash(uint32_t argMode)
{
    uint32_t readByte = 0, remainByte = 0, flashArgAddr = 0;
    uint8_t *data = NULL;
    Reg *reg;
    int i = 0;

    data = (uint8_t *)pvPortMalloc(sizeof(g_saveRegs));
    if (data == NULL)
    	return;

    remainByte = sizeof(g_saveRegs);
    flashArgAddr = g_regFlashAddr[argMode];

    for (i = 0; i < sizeof(g_saveRegs); i += NOR_FLASH_RW_MAX_SIZE) {
        readByte = remainByte < NOR_FLASH_RW_MAX_SIZE ? remainByte : NOR_FLASH_RW_MAX_SIZE;
        NorFlashReadBuf(SPI_DEV_ID_CODE, flashArgAddr + i, readByte, data + i);

        remainByte -= readByte;
    }

    /* stop image stream */
	WriteRegByAddr(AXI_BASE_ADDR + IMAGE_CAPTURE, 0);
	DelayMs(1000);
    for (i = sizeof(g_saveRegs) / sizeof(g_saveRegs[0]) - 2; i >= 0; i --) {
    	reg = (Reg *)(data + i * sizeof(Reg));
    	if (reg->addr != g_saveRegs[i].addr || reg->value == 0xFFFFFFFF) {
    		break;
    	}

        g_saveRegs[i].value = reg->value;
        WriteReg(&g_saveRegs[i]);
    }

    DelayMs(5000);
    int tmp = sizeof(g_saveRegs) / sizeof(g_saveRegs[0]) - 1;
    reg = (Reg *)(data + tmp * sizeof(Reg));
	if (reg->addr == g_saveRegs[tmp].addr && reg->value != 0xFFFFFFFF) {
		g_saveRegs[tmp].value = reg->value;
		WriteReg(&g_saveRegs[tmp]);
	}

    PrintBuf(g_saveRegs, sizeof(g_saveRegs), "load args g_saveRegs");

    vPortFree(data);
}
#else
void LoadArgsFromFlash(uint32_t argMode)
{
    uint32_t readByte = 0, remainByte = 0, i = 0, flashArgAddr = 0;
    uint8_t *data = NULL;
    Reg *reg = NULL;

    remainByte = sizeof(g_saveRegs);
    data = malloc(remainByte);
    memset(data, 0x00, sizeof(remainByte));
    flashArgAddr = g_regFlashAddr[argMode];

    for (i = 0; i < sizeof(g_saveRegs); i += NOR_FLASH_RW_MAX_SIZE) {
        readByte = remainByte < NOR_FLASH_RW_MAX_SIZE ? remainByte : NOR_FLASH_RW_MAX_SIZE;
        NorFlashReadBuf(SPI_DEV_ID_CODE, flashArgAddr + i, readByte, data + i);

        remainByte -= readByte;
    }

    for (i = 0; i < sizeof(g_saveRegs) / sizeof(g_saveRegs[0]); i ++) {
    	reg = (Reg*)(data + i * sizeof(Reg));
        WriteReg(reg);
        LOG_DEBUG("write regs, a: 0x%x, d: 0x%x\r\n", reg->addr, reg->value);
        DelayMs(2); /* wait for FPGA */
    }

    free(data);
}
#endif

void InitWriteCameraRegListToFlash(void)
{
	uint32_t reg; /* check the first addr */

	for (int i = ARG_MODE_MAX - 1; i >= 0; i --) {
	    /* Check for register data in flash */
		NorFlashReadBuf(SPI_DEV_ID_CODE, g_regFlashAddr[i],	sizeof(reg), (uint8_t *)&reg);
		if (reg == 0xFFFFFFFF) {
			LOG_DEBUG("There is no reg data in flash, so write reg data into flash");
			/* read reg */
			for (int j = 0; j < REG_LIST_TOTAL_NUM; j ++) {
				ReadReg(&g_saveRegs[j]);
			}
			SaveArgsToFlash(i);
		}
	}
}

#else
void HandleSaveArgs(SerialData *receivedData){}
void SaveArgsToFlash(uint32_t argMode){}
void LoadArgsFromFlash(uint32_t argMode){}
void InitWriteCameraRegListToFlash(void){}
#endif /* MODULE_SPI_FLASH || MODULE_UART */

#ifdef MODULE_UART
void HandleSetUartBaudrate(SerialData *recvData)
{
    if (!recvData)
        return;

    Pack *pkt = (Pack *)recvData->data;
    int dataLen = (int)pkt->dataLen;
    uint8_t *data = pkt->dataPayload;
    uint16_t crc = (*(data + dataLen)) + ((*(data + dataLen + 1)) << 8);
    uint32_t baudrate = 0;

    /* crc */
    if (!CheckCrc16(recvData->data, recvData->size - 2, crc)) {
        LOG_DEBUG("check crc error\r\n");
    /* send error packet */
        return;
    }

    baudrate = *(uint32_t *)(data + 1);

    if (baudrate > BAUD_460800_VALUE)
        return;

    UartInit(UART_UPDATE, &g_uart[UART_UPDATE], &g_uartConfig[UART_UPDATE], baudrate);
}
#else
void HandleSetUartBaudrate(SerialData *recvData){}
#endif
void HandleSaveCsv(SerialData *recvData)
{
	/*Whether RecvData is Null*/
    if (!recvData)
        return;

    /*Init variable*/
    Pack *pkt = (Pack *)recvData->data;
    int dataLen = (int)pkt->dataLen;
    uint8_t *data = pkt->dataPayload;
    uint16_t seqNum = pkt->seqNum;
    uint16_t crc = 0;
    uint32_t blockStartAddr = FLASH_SPACE_CSV_START_ADDR;
    uint32_t currentAddress = FLASH_SPACE_CSV_START_ADDR + (seqNum - 1) * (PACKET_DATA_MAX_LENGTH - 1);
    MessageReplyPack parse;
    parse.cmdType = CMD_TYPE_SAVE_CSV;
    memcpy(&crc,data+dataLen,sizeof(crc));
    /* CRC check*/
    if (!CheckCrc16(recvData->data, recvData->size - 2, crc)) {
        parse.cmdStatus = HANDLE_CSV_CRC_ERROR;
        LOG_DEBUG("check crc error in HandleSaveCsv\r\n");
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&parse, CSV_SAVEMESSAGE_LENGTH, 0);
        return;
    }
    /* Handle information Packet*/
    if (seqNum == 0) {
        LOG_DEBUG("save csv messege successful\r\n");
        parse.cmdStatus = HANDLE_CSV_FINISH;
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&parse, CSV_SAVEMESSAGE_LENGTH, 0);
        NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_ARG, blockStartAddr);
        DelayMs(5);
        NORFLASH_ERASE_4K_SECTOR(SPI_DEV_ID_ARG, blockStartAddr + 0x1000);
        DelayMs(5);
        return;
    }
    dataLen -= 1;
    data += 1;
    /*Check length Whether right*/
    if (dataLen <= 0){
        parse.cmdStatus = HANDLE_CSV_LENGTH_ERROR;
        LOG_DEBUG("Check dataLen error in HandleSaveCsv\r\n");
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&parse, CSV_SAVEMESSAGE_LENGTH, 0);
        return;
    }
    NorFlashWriteData(SPI_DEV_ID_ARG, currentAddress, data, dataLen);
    LOG_DEBUG("HandleSaveCSv test successful\r\n");
    parse.cmdStatus = HANDLE_CSV_FINISH;
    SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&parse, CSV_SAVEMESSAGE_LENGTH, 0);
}
void HandleSendCsv(SerialData *recvData)
{
    if (!recvData)
    	return;

    Pack *pkt = (Pack *)recvData->data;
    int dataLen = (int)pkt->dataLen;
    int remain = 0;
    int  offset = 0;
    int readBytes = 0;
    int i = 0;
    uint8_t *data = pkt->dataPayload;
    uint32_t blockStartAddr = FLASH_SPACE_CSV_START_ADDR;
    uint8_t packetData[PACKET_DATA_MAX_LENGTH] = {0};
    MessageReplyPack startReply;
    MessageReplyPackWithPackNum Reply;
    uint8_t *dataBuf;
    uint32_t packetLen = 0;
    uint32_t fileSize = 0;
    uint8_t sizeBuf[sizeof(fileSize)];
    uint16_t crc = 0;
    /*CRC check*/
    memcpy(&crc,data+dataLen,sizeof(crc));
    if (!CheckCrc16(recvData->data, recvData->size - 2, crc)) {
        LOG_DEBUG("Check CRC Error in HandleSendCsv\r\n");
        startReply.cmdType = CMD_TYPE_SEND_CSV;
        startReply.cmdStatus = HANDLE_CSV_CRC_ERROR;
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&startReply, sizeof(startReply), 0);
        return;
    }
    /*DataLen Check*/
    if (dataLen != 1) {
        LOG_DEBUG("Check Data Error in HandleSendCsv\r\n");
        startReply.cmdType = CMD_TYPE_SEND_CSV;
        startReply.cmdStatus = HANDLE_CSV_LENGTH_ERROR;
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&startReply, sizeof(startReply), 0);
        return;
    }
    /* Read Size of file*/
    for ( i = 0; i < sizeof(fileSize); i++) {
        NorFlashReadBuf(SPI_DEV_ID_ARG, blockStartAddr + i, 1, sizeBuf + i);
        DelayMs(2);
    }
    memcpy(&fileSize,sizeBuf,sizeof(sizeBuf));
    LOG_DEBUG("filesize:%4x\r\n", fileSize);
    if (fileSize == 0) {
        LOG_DEBUG("No Data in Flash in HandleSendCsv\r\n");
        startReply.cmdType = CMD_TYPE_SEND_CSV;
        startReply.cmdStatus = HANDLE_CSV_FILLSIZE_ERROR;
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&startReply, sizeof(startReply), 0);
        return;
    }
    /*distribution space of file*/
    dataBuf = (uint8_t *)calloc(fileSize, sizeof(uint8_t));
    if (dataBuf == NULL) return;
    remain = fileSize;
    for (offset = 0; offset < fileSize; offset += NOR_FLASH_RW_MAX_SIZE) {
        readBytes = (remain < NOR_FLASH_RW_MAX_SIZE) ? remain : NOR_FLASH_RW_MAX_SIZE;
        NorFlashReadBuf(SPI_DEV_ID_ARG, blockStartAddr + offset, readBytes, dataBuf + offset);
        DelayMs(3);
        remain -= readBytes;
    }
    /*calculate  and reply file information*/
    int numPackets = (fileSize + PACKET_DATA_MAX_LENGTH - 1) / PACKET_DATA_MAX_LENGTH;
    Reply.cmdType = CMD_TYPE_SEND_CSV;
    Reply.csvStatus = HANDLE_CSV_FINISH;
    Reply.packNum  = numPackets;
    SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, (uint8_t *)&Reply, sizeof(Reply), 0);
    LOG_DEBUG("Send Messagereply successful in HandleSendCsv\r\n");
    DelayMs(10);
    /* Send data Packet */
    for (i = 0; i < numPackets; i++) {
        offset = i * PACKET_DATA_MAX_LENGTH;
        packetLen = fileSize - offset;
        if (packetLen > PACKET_DATA_MAX_LENGTH) {
            packetLen = PACKET_DATA_MAX_LENGTH;
        }
        memcpy(packetData, dataBuf + offset, packetLen);
        SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, packetData, packetLen, i + 1);
        DelayMs(20);
    }
    free(dataBuf);
    LOG_DEBUG("HandleSendCsv run finished\r\n");
}

void HandleSendVersion(SerialData *recvData)
{
	if(!recvData)
		return;
	Pack *pkt = recvData -> data;
	MessageReplyPack reply;
	SendVersionPack versionBuf;
	uint8_t *data = pkt-> dataPayload;
	int dataLen = (int)pkt->dataLen;
	uint8_t buf[] = VERSION;
	uint8_t fpgabuf[] = "PV3412-M.V01GR01";
	uint16_t crc = 0;
	memcpy(&crc,data+dataLen,sizeof(crc));
	if (!CheckCrc16(recvData->data, recvData->size - 2, crc))
	{
		LOG_DEBUG("Check CRC Error in HandleSendCsv\r\n");
		reply.cmdType = CMD_TYPE_SEND_VERSION;
		reply.cmdStatus = HAND_SEND_VERSION_ERROR;
		SendPacket(UART_CMD, PACKET_TYPE_CMD_REPLY, &reply, sizeof(reply), 0);
		return;
	 }
	/* microblaze version */
	memset(&versionBuf, 0x00, sizeof(versionBuf));
	versionBuf.cmdType = CMD_TYPE_SEND_VERSION;
	versionBuf.cmdStatus = HAND_SEND_VERSION_FINISH;
	strcat(&versionBuf.cmdVersion,buf);
	strcat(&versionBuf.cmdVersion,"|");
	NorFlashReadBuf(SPI_DEV_ID_CODE,FPGA_VERSION_START_ADDR,FPGA_VERSION_LEN, fpgabuf);
	DelayMs(2);
	strcat(&versionBuf.cmdVersion,fpgabuf);
	SendPacket(UART_CMD,PACKET_TYPE_CMD_REPLY,(uint8_t *)&versionBuf,sizeof(versionBuf),0);
	if (UART_CMD != UART_UPDATE) {
		UartSend(UART_UPDATE, &versionBuf, (int)strlen(&versionBuf));
	}
	LOG_DEBUG("fpga version: %s\r\n", versionBuf);
}
#ifdef MODULE_UART
void CmdTask(void *unused)
{
    SerialData receivedData;
    Pack *pkt = NULL;
    uint8_t cmdType = 0;
    while (1) {
        if (xQueueReceive(g_queueUart[QUEUE_UART_CMD].queue, &receivedData, pdMS_TO_TICKS(100))) {
            pkt = (Pack *)receivedData.data;
            cmdType = pkt->dataPayload[0];
            if (cmdType > 0 && cmdType < CMD_TYPE_NUM_MAX) {
                cmdFunc[cmdType](&receivedData);
            }
        }
#ifdef MODULE_I2C
        I2cSlaveCmd();
#endif
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
#else
void CmdTask(void *unused){}
#endif
