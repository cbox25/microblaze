#include "uart.h"
#include "packet_sr.h"
#include "firmware_upgrade.h"

/* Send a response packet for each file data packet. */
int SendPacket(int uartNum, uint8_t type, uint8_t *buffer, uint16_t length, uint16_t SeqNum)
{
	uint32_t pktInfoLen = 0;
	uint16_t crc16 = 0;
	Pack pkt;
	pkt.startFlag = ConvertEndian16(PACKET_START_FLAG);
	pkt.type = type;
	pkt.compressType = COMPRESS_TYPE_NONE;
	pkt.seqNum = SeqNum;
	pkt.dataLen = length;
	memcpy(pkt.dataPayload, buffer, length);
	pktInfoLen = (uint8_t *)(&pkt.dataPayload) - (uint8_t *)(&pkt.startFlag);
	crc16 = CalcCrc16((uint8_t *)&pkt, pktInfoLen + length);
	memcpy(pkt.dataPayload + length, &crc16, sizeof(crc16));
	UartSend(uartNum, (uint8_t *)&pkt, pktInfoLen + length + sizeof(crc16));

	return 0;
}

/* Construct the data portion of a register read/write response packet. */
uint32_t GetRegData(uint8_t *data, Pack *packet, uint8_t rwStatus, uint32_t errorCode, uint8_t *readData)
{
    uint32_t length = 0; /* Data length */
    uint32_t rwLength = 0;

    /* Operation Type */
    memcpy(data, packet->dataPayload, 5);
    length += 5;

    /* Read/Write Status */
    data[5] = rwStatus;
    length++;

    /* Error code */
    memcpy(data + length, &errorCode, sizeof(errorCode));
    length += 4;

    /* Operating length */
    memcpy(data + length, packet->dataPayload + 5, 4); /* FIXME */
    length += 4;

    rwLength = *(uint32_t *)(data + 10);

    /* Here, it determines whether to populate the data section only if the operation is correct and it is a read command. */
    if ((data[0] == READ_REG) && (data[5] == RW_REG_SUCCESS)) {
        memcpy(data + 14, readData, rwLength);
        length += rwLength;
    }

    return length;
}
uint32_t GetHandShakeData(uint8_t *data, uint8_t handShakeStatus, uint32_t reason)
{
    data[0] = handShakeStatus;
    memcpy(&data[1], &reason, sizeof(reason));
    return 5;
}

/* Construct the data portion of a response packet for firmware upgrade file data. */
uint32_t GetUpgradeData(uint8_t *data, uint8_t replyResult, uint32_t errorCode)
{
    data[0] = replyResult;
    memcpy(&data[1], &errorCode, sizeof(errorCode));
    return 5;
}

/* Convert the data packet into a structured format */
void ParseUartPacket(uint8_t *buffer, Pack *packet)
{
    uint8_t len = (uint8_t *)(packet->dataPayload) - (uint8_t *)(&packet->startFlag);
    memcpy(packet, buffer, len);


    if (packet->dataLen > PACKET_DATA_MAX_LENGTH) {
        return; /* this packet data is error*/
    }

    memcpy(packet->dataPayload, buffer + len, packet->dataLen);
    memcpy(&packet->crc16, &buffer[len + packet->dataLen], sizeof(packet->crc16));
}
