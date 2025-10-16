#include <operate_reg.h>
#include "Xil_io.h"
#include "cmd.h"

/* If it is a command to read a specified address register */
void ReadVideoReg(Pack *packet)
{
    Reg reg;
    uint32_t regAddr, regLen = 0, length = 0, i = 0;
    uint8_t readData[PACKET_DATA_MAX_LENGTH];
    uint8_t data[PACKET_DATA_MAX_LENGTH];

    /* Obtain the length of the data to be read */
    memcpy(&regAddr, packet->dataPayload[1], sizeof(regAddr));
    memcpy(&regLen, packet->dataPayload[5],sizeof(regLen));

    /* Retrieve register values from a specified address */
    for (i = 0; i < regLen / 4; i ++) {
        reg.addr = regAddr;
        reg.value = 0;
        ReadReg(&reg);
        memcpy(readData + i, &reg.value, sizeof(reg.value));
        regAddr += 4;
        LOG_DEBUG("a: 0x%x, v: %d\r\n", reg.addr, reg.value);
    }

    length = GetRegData(data, packet, RW_REG_SUCCESS, SUCCESS, readData);

#ifdef MODULE_UART
    SendPacket(UART_CMD, PACKET_TYPE_REG_REPLY, data, length, 0);
#endif
}

void OperateReg(SerialData *receivedData)
{
    Pack packet;
    Reg algrithmReg;
    int len = 0;
    uint8_t data[PACKET_DATA_MAX_LENGTH];
    uint32_t length = 0, writeValue = 0;

    /* First, convert the data into a structure. */
    len = (uint8_t *)packet.dataPayload - (uint8_t *)&packet.startFlag;
    memcpy(&packet, receivedData->data, len);
    if (packet.dataLen > PACKET_DATA_MAX_LENGTH) {
    	LOG_DEBUG("check length error\r\n");
		length = GetRegData(data, &packet, RW_REG_FALSE, RW_REG_LENGTH_ERROR, 0);

#ifdef MODULE_UART
		SendPacket(UART_CMD, PACKET_TYPE_REG_REPLY, data, length, 0);
#endif
        return; /* this packet data is error*/
    }

    memcpy(&packet.dataPayload, receivedData->data + len, packet.dataLen);
    memcpy(&packet.crc16, receivedData->data + len + packet.dataLen, sizeof(packet.crc16));
    /*If CRC check fails, send a CRC check failure packet.*/
    if (!CheckCrc16(receivedData->data, receivedData->size - sizeof(packet.crc16), packet.crc16)) {
        LOG_DEBUG("check crc error\r\n");
        /*Construct the response data portion for register read/write operations*/
        length = GetRegData(data, &packet, RW_REG_FALSE, RW_REG_CRC_ERROE, 0);

#ifdef MODULE_UART
        SendPacket(UART_CMD, PACKET_TYPE_REG_REPLY, data, length, 0);
#endif
        return;
    }

    /*If the CRC check succeeds, distribute to different functions based on the operation type*/
    /*If it is a read operation*/
    if (packet.dataPayload[0] == READ_REG) {
        ReadVideoReg(&packet);
    } else if (packet.dataPayload[0] == WRITE_REG) {
        /*If it's a write operation*/
        algrithmReg.addr = *(uint32_t *)(packet.dataPayload + 1);
        algrithmReg.value = *(uint32_t *)(packet.dataPayload + 5);
        algrithmReg.delay = *(uint32_t *)(packet.dataPayload + 9);
        LOG_DEBUG("write: a: 0x%08X, d: 0x%08X\r\n", algrithmReg.addr, algrithmReg.value);
        WriteReg(&algrithmReg);
        algrithmReg.value = 0;
        ReadReg(&algrithmReg);
        LOG_DEBUG("read : a: 0x%08X, d: 0x%08X\r\n", algrithmReg.addr, algrithmReg.value);
        if (writeValue == algrithmReg.value){
        	LOG_DEBUG("write reg OK\r\n");
			length = GetRegData(data, &packet, RW_REG_SUCCESS, SUCCESS, 0);
        } else{
        	LOG_DEBUG("write reg error\r\n");
        	length = GetRegData(data, &packet, RW_REG_FALSE, WRITE_REG_ADDRESS_IS_BAN, 0);
        }

#ifdef MODULE_UART
        SendPacket(UART_CMD, PACKET_TYPE_REG_REPLY, data, length, 0);
#endif
    } else {
        /*If it's an unknown type, we'll ignore it for now and not respond.*/
        return;
    }
}
