#ifndef  FIRMWARE_UPDATE_H
#define  FIRMWARE_UPDATE_H

#include "uart.h"
#include "FreeRTOS.h"
#include "packet_sr.h"
#include "common.h"

#ifdef MODULE_SPI_FLASH

enum {
    UPDATE_TYPE_FULL = 0x01,
    UPDATE_TYPE_INCREASE = 0x02
};

enum {
    COMPRESS_TYPE_NONE = 0,
    COMPRESS_TYPE_LZ4 = 1
};

enum {
    FPGA_UPDATE_FILETYPE = 0x00,
    GENCP_XML_FILETYPE = 0x01
};

/*Here we define a structure to describe the transmission status.*/
typedef struct {
    uint8_t isHandShake; /*0 indicates no handshake； 1 indicates successful handshake*/
    uint32_t connectStatus; /*Connection Status: 0 indicates no transmission in progress; 1 indicates transmission in progress; 2 indicates host computer reset request; 3 indicates transmission completed.*/
    uint32_t fileSize; /*File size of this transmission*/
    uint16_t recvPacketNum; /*Total number of packets to be received this time*/
    uint16_t packetCnt; /*Number of packets currently transmitted*/
    uint8_t    updateType; /* Online upgrade types: 1: Incremental update 2: Full update*/
    uint8_t compressType; /* 1: not compressed, 2: compress */
    uint32_t flashAddr; /* the address of writing flash */
} Upgrade;

extern Upgrade g_upgrade; /*Define a handshake state instance*/

int ConnectToPc(SerialData *receivedData);
int ReceiveFile(SerialData *receivedData);
void DoFirmwareUpgrade(SerialData *receivedData);
void FirmwareUpdateTask(void *unused);

void ClearUpgradeStatus(Upgrade *upgrade); /*Transmission Status Reset*/
#endif /* MODULE_SPI_FLASH */
#endif /* FIRMWARE_UPDATE_H */
