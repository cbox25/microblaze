#ifndef PACKET_SR_H
#define PACKET_SR_H
#include <unistd.h>
#include "crc.h"
#include "flash.h"

#include "common.h"
/*To describe a data packet, construct a structure.*/
typedef struct __attribute__((packed))
{
    uint16_t startFlag;
    uint8_t type;
    uint8_t compressType;
    uint8_t reserved[2];
    uint16_t seqNum;
    uint16_t dataLen;
    uint8_t dataPayload[PACKET_DATA_MAX_LENGTH];
    uint16_t crc16;
} Pack;

typedef struct {
    uint32_t size;
    uint8_t data[PACKET_MAX_LENGTH];
} SerialData;

/*Packet sending soft-core response data packets*/
int SendPacket(int uartNum, uint8_t packettype, uint8_t *buffer, uint16_t length, uint16_t seqNum);

/*Convert the data packet into a structured format*/
void ParseUartPacket(uint8_t *buffer, Pack *packet);

/*Construct the data portion of a register read/write response packet.*/
/*
 * 参数：data    The array returned as the return value
 *             packet    Data packet requiring acknowledgment from PC to Microblaze
 *             operateStatus    Operation status
 *             errorCode            Error code
 *             readData        Data read from the register
 */
    uint32_t GetRegData(uint8_t *data, Pack *packet, uint8_t rwStatus, uint32_t errorCode, uint8_t *readData);
/*Construct the data portion of a handshake response packet*/
/*
 * Parameter: data  as the array returned
 *             handShakeStatus    Handshake Status Indicator
 *             reason    Reasons for Disconnecting/Refusing Handshake
 */
uint32_t GetHandShakeData(uint8_t *data, uint8_t handShakeStatus, uint32_t reason);

/*Construct the data portion of a firmware upgrade response packet*/
/*
 * Parameter: data  as the array returned
 *             replyResult    Handshake Status Indicator
 *             errorCode    Reasons for Disconnecting/Refusing Handshake
 */
uint32_t GetUpgradeData(uint8_t *data, uint8_t replyResult, uint32_t errorCode);

#endif
