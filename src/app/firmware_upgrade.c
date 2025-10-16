#include "common.h"
#include "firmware_upgrade.h"
#include "xtmrctr.h"
#include "FreeRTOS.h"
#include "packet_sr.h"
#include "lz4.h"

#ifdef MODULE_SPI_FLASH
Upgrade g_upgrade;
int g_firmwareUpdateTaskDelay = 300;

void ClearUpgradeStatus(Upgrade *upgrade)
{
    upgrade->isHandShake = 0;
    upgrade->connectStatus = UN_TRANSMIT;
    upgrade->fileSize = 0;
    upgrade->recvPacketNum = 0;
    upgrade->packetCnt = 0;
    upgrade->updateType = UPDATE_TYPE_FULL; /* default use full update */
}

/* The minimum size of erasing nor flash is 4KB, so before rewriting each sub-packet data of 128B,
   the entire block with size of 4KB should be erased.
   erase block: 4KB,
   receive data packet: 512B,
   read or write nor flash packet: 128B */
static uint8_t g_updateBlockBuf[NOR_FLASH_ERASE_BLOCK_SIZE_4K];

/* full update */
int FirmwareUpdateWriteFlash(Pack *packet)
{
    uint32_t currentAddress = 0;
    int ret = 0,  decompressLen = 0;
    uint8_t decompressBuf[PACKET_DATA_MAX_LENGTH];

    if (packet == NULL)
        return -1;

    currentAddress = (packet->seqNum - 1) * PACKET_DATA_MAX_LENGTH + g_upgrade.flashAddr; /*FLASH_SPACE_FIRMWARE_UPDATE_START_ADDR;*/
    /* check the compress type, and decompress data of packet->dataPayload */
    memset(decompressBuf, 0x00, sizeof(decompressBuf));
    if (packet->compressType == COMPRESS_TYPE_NONE) {
        memcpy(decompressBuf, packet->dataPayload, packet->dataLen);
        decompressLen = packet->dataLen;
    } else if (packet->compressType == COMPRESS_TYPE_LZ4) {
        decompressLen = LZ4_decompress_safe((const char *)(packet->dataPayload), (char *)decompressBuf,
                                            packet->dataLen, PACKET_DATA_MAX_LENGTH);
        if (decompressLen < 0) {
            LOG_DEBUG("decompress failed, len: %d, seq: %u\r\n", decompressLen, packet->seqNum);
            return -2;
        }
    }

    /* write data info flash */
    ret = NorFlashWriteData(SPI_DEV_ID_CODE, currentAddress, decompressBuf, decompressLen);
    if (ret != 0) {
        return -3;
    }

    return 0;
}

int ConnectToPc(SerialData *receivedData)
{
    /*Convert the data packet into a pack structure*/
    Pack packet;
    ParseUartPacket(receivedData->data, &packet);
    uint8_t data[PACKET_DATA_MAX_LENGTH];
    uint32_t length;

    /*Determine whether it is a handshake packet. If not, discard it directly*/
    if (packet.type != PACKET_TYPE_HANDSHAKE) {
        LOG_DEBUG("handshake type %d err\r\n", packet.type);
        return -1;
    }

    /*Perform CRC check*/
    /*If CRC check fails, send a CRC check failure packet*/
    if (!CheckCrc16(receivedData->data, receivedData->size - 2, packet.crc16)) {
        /*Construct the response data portion for a handshake*/
        length = GetHandShakeData(data, HANDSHAKE_MICROBLAZE_REJECT, HANDSHAKE_CRC_ERROR);
        SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);
        LOG_DEBUG("handshake crc err, len: %u\r\n", receivedData->size);
        return -2;
    }
    /*Determine if it is a request connection in progress*/
    /*If not, ignore other commands when no connection has been established*/
    if (packet.dataPayload[0] == HANDSHAKE_PC_DISCONNECT) {
        length = GetHandShakeData(data, HANDSHAKE_MICROBLAZE_DISCONNECT, HANDSHAKE_PC_DISCONNECT_ACK);
        SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);
        LOG_DEBUG("handshake disconnect err\r\n");
        return -3;
    }

    /*Send and receive handshake packets*/
    /*Construct the response data portion of a handshake*/
    length = GetHandShakeData(data, HANDSHAKE_ACCEPT, 0);
    SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);

    LOG_DEBUG("handshake OK\r\n");

    /*Currently in handshake state*/
    g_upgrade.isHandShake = 1;
    return SUCCESS;
}

/*Receive file*/
int ReceiveFile(SerialData *receivedData)
{
    /*Indicates the status of each operation*/
    /*Used to represent the data portion and length of the response packet*/
    uint8_t data[PACKET_DATA_MAX_LENGTH];
    uint32_t length = 0, flashAddress = 0;
    static uint16_t lastSeq = 0;
    uint8_t fileType = 0, updateType = 0;
    int ret = 0;

    /*Convert the byte array data packet into a structured format.*/
    Pack packet;
    /* Convert the package to a struct*/
    ParseUartPacket(receivedData->data, &packet);

    /*If it's a handshake packet, we determine what the handshake packet is doing at this point.*/
    if (packet.type == PACKET_TYPE_HANDSHAKE) {
        /*Perform CRC check first*/
        /*If CRC check fails, send a CRC check failure packet*/
        if (!CheckCrc16(receivedData->data, receivedData->size - 2, packet.crc16)) {
            /*Construct the response data portion for a handshake*/
            length = GetHandShakeData(data, HANDSHAKE_MICROBLAZE_REJECT, HANDSHAKE_CRC_ERROR);
            SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);
            return -1;
        }

        /*When we are in a connected state and the PC requests a connection again, we reject it.*/
        if (packet.dataPayload[0] == HANDSHAKE_CONNECT) {
            length = GetHandShakeData(data, HANDSHAKE_MICROBLAZE_REJECT, HANDSHAKE_MICROBLAZE_SHAKING);
            SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);
            return -2;
        }

        /*If the host computer requests a disconnect, we disconnect.*/
        if (packet.dataPayload[0] == HANDSHAKE_PC_DISCONNECT) {
            length = GetHandShakeData(data, HANDSHAKE_MICROBLAZE_DISCONNECT, HANDSHAKE_PC_DISCONNECT_ACK);
            SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);
            return PC_REQUEST_RESET;
        }
        LOG_DEBUG("handshake OK\r\n");
    } else if (packet.type == PACKET_TYPE_UPDATE_DATA) {
		/*Perform CRC check on file data packets*/
		/*If the CRC check fails, a packet indicating that the file data packet has failed CRC check is sent.*/
		if (!CheckCrc16(receivedData->data, receivedData->size - 2, packet.crc16)) {
			/*Construct the reply data portion of a data packet*/
			length = GetUpgradeData(data, UPDATE_PACKET_ERROR, UPDATE_PACKET_CRC_ERROR);
			SendPacket(UART_UPDATE, PACKET_TYPE_UPDATE_REPLY, data, length, packet.seqNum);
			return -4;
		}

		if (packet.seqNum == 0) {
			LOG_DEBUG("file info packet start\r\n");
			g_upgrade.recvPacketNum = packet.dataPayload[0] + (packet.dataPayload[1] << 8);
			g_upgrade.fileSize = packet.dataPayload[2] + (packet.dataPayload[3] << 8) + (packet.dataPayload[4] << 16) + (packet.dataPayload[5] << 24);
			g_upgrade.packetCnt = 0; /* reset packet count
			updateType = packet.dataPayload[6];
			fileType = packet.dataPayload[7];
			lastSeq = 0;
			/*According to the file information package, we erase the Flash and erase all of it here.*/
			/* StartTimer(xMyTimer0);
			/*Calculate how many sectors to erase based on file size*/
			uint32_t eraseSectorNum = (g_upgrade.fileSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
			if (fileType == FPGA_UPDATE_FILETYPE) {
				g_upgrade.flashAddr = FLASH_SPACE_FIRMWARE_UPDATE_START_ADDR;
			} else if(fileType == GENCP_XML_FILETYPE) {
				g_upgrade.flashAddr = FLASH_SPACE_GENCP_XML_INFO_START_ADDR;
			}
			flashAddress = g_upgrade.flashAddr;
			LOG_DEBUG("erase sectors: %u, filesize: %u\r\n", eraseSectorNum, g_upgrade.fileSize);
			LOG_DEBUG("packet num: %u\r\n", g_upgrade.recvPacketNum);
			if (updateType == UPDATE_TYPE_FULL) {
				LOG_DEBUG("ERASE\r\n");
				/* only full update erase flash here */
				for (int i = 0; i < eraseSectorNum; i++) {
					NORFLASH_ERASE_SECTOR(SPI_DEV_ID_CODE, flashAddress);
					flashAddress += SECTOR_SIZE;
				}
			} else if (updateType == UPDATE_TYPE_INCREASE) {
				;
			}

			/* When file info packet has been handled with, send ack packet */
			length = GetUpgradeData(data, UPDATE_PACKET_ACK, 0);
			SendPacket(UART_UPDATE, PACKET_TYPE_UPDATE_REPLY, data, length, packet.seqNum);
			LOG_DEBUG("file info packet OK\r\n");
		} else {
			if (lastSeq != packet.seqNum) {
				if (g_upgrade.updateType == UPDATE_TYPE_FULL) {
					ret = FirmwareUpdateWriteFlash(&packet);
				} else {
					;
				}
			}

			/* Data packet processing completed. Send response acknowledgment.*/
			 /* Construct the reply data portion of a data packet. */
			if (ret == 0) {
				length = GetUpgradeData(data, UPDATE_PACKET_ACK, 0);
			} else {
				length = GetUpgradeData(data, UPDATE_PACKET_WRITE_FLASH_ERROR, ~ret);
			}
			SendPacket(UART_UPDATE, PACKET_TYPE_UPDATE_REPLY, data, length, packet.seqNum);
			LOG_DEBUG("seq: %u, recved: %u / %u\r\n", packet.seqNum, g_upgrade.packetCnt, g_upgrade.recvPacketNum);
		}

		if (lastSeq != packet.seqNum) {
			g_upgrade.packetCnt++; /* file info packet does not +1 */
			lastSeq = packet.seqNum;
		}

		/* After the package is collected, you can perform operations such as returning data. */
		if (g_upgrade.packetCnt == g_upgrade.recvPacketNum) {
			LOG_DEBUG("update OK\r\n");
			return TRANSMIT_FINSH;
		}

		return TRANSMITTING;
	}
}

/* Initiate firmware upgrade */
void DoFirmwareUpgrade(SerialData *receivedData)
{
    /* Used to indicate the data portion and length of the response packet */
    uint8_t data[PACKET_DATA_MAX_LENGTH];
    uint32_t length;
    int status;

    /* If no handshake has been established, we prioritize entering the handshake determination zone.
     * Before establishing a handshake, any firmware upgrade data packets are handled by the handshake determination function. */
    if (g_upgrade.isHandShake == 0) {
        LOG_DEBUG("connect to pc\r\n");
        status = ConnectToPc(receivedData);
        /* If the handshake fails, ignore it. The function will respond. */
        if (status != SUCCESS) {
            /* Print the reason for the failed handshake here */
            LOG_DEBUG("HANDSHAKE_FALSE, err code: %d\r\n", status);
        }
        return;
    }

    /* At this point, the handshake is complete.
     * Begin receiving the file. */
    status = ReceiveFile(receivedData);
    if (status == TRANSMITTING) {
    	/* 1 indicates transmission in progress */
        ;
    } else if (status == PC_REQUEST_RESET) {
    	/* 2 indicates the host computer has requested a reset. */
        LOG_DEBUG("pc request reset\r\n");
        /* Clear the current transfer status */
        ClearUpgradeStatus(&g_upgrade);
    } else if (status == TRANSMIT_FINSH) {/*Transmission complete*/
        LOG_DEBUG("transmit finished\r\n");
        g_firmwareUpdateTaskDelay = 300;
        /* Disconnect from the host computer */
        length = GetHandShakeData(data, HANDSHAKE_MICROBLAZE_DISCONNECT, HANDSHAKE_TRANSMISSION_COMPLETE);
        SendPacket(UART_UPDATE, PACKET_TYPE_HANDSHAKE_REPLY, data, length, 0);
        /* Clear the current transfer status */
        ClearUpgradeStatus(&g_upgrade);
    }
}

void FirmwareUpdateTask(void *unused)
{
    SerialData receivedData;
    TickType_t lastRecvTick = xTaskGetTickCount();

    while(1) {
        if (g_upgrade.isHandShake == 1) {
            /* if it has been more than 30s since the last received data packet,
             * restore the task delay time to be 300ms */
            if ((xTaskGetTickCount() - lastRecvTick) > pdMS_TO_TICKS(30000) && g_uartConfig[UART_UPDATE].baudRate != 115200) {
                g_firmwareUpdateTaskDelay = 300;
                UartInit(UART_UPDATE, &g_uart[UART_UPDATE], &g_uartConfig[UART_UPDATE], BAUD_115200_VALUE);
                ClearUpgradeStatus(&g_upgrade);
                LOG_DEBUG("restore baudrate as 115200\r\n");
            }
        }

        if (xQueueReceive(g_queueUart[QUEUE_UART_FIRMWARE_UPDATE].queue, &receivedData, pdMS_TO_TICKS(100))) {
            DoFirmwareUpgrade(&receivedData);
            lastRecvTick = xTaskGetTickCount();
            g_firmwareUpdateTaskDelay = 10;
        } else if (xQueueReceive(g_queueUart[QUEUE_UART_HANDSHAKE].queue, &receivedData, pdMS_TO_TICKS(100))) {
            lastRecvTick = xTaskGetTickCount();
            LOG_DEBUG("upgrade handshake buf\r\n");
            DoFirmwareUpgrade(&receivedData);
            g_firmwareUpdateTaskDelay = 10;
        } else {
            vTaskDelay(pdMS_TO_TICKS(g_firmwareUpdateTaskDelay));
        }
    }
}
#endif /* MODULE_SPI_FLASH */
