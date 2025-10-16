#include "sa_mf210a.h"
#include <string.h>
#include "uart.h"

#ifdef MOUDULE_SA_MF210A
typedef int (*Sa_Mf201a_Cb)(SerialData *recvData);

Sa_Mf201a_Cb cmdCallBack[CMD_NUM_MAX] = {
        NULL,
		HandleStartDetectionFrame,
		HandleSetThresholdFrame,
};
#endif /* MODULE_UART */

/* ========================================================================== */
/* CRC16-MODBUS (poly 0xA001, init 0xFFFF, no reflection, XOROUT 0x0000)      */
/* ========================================================================== */

uint16_t CalculateCrc(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFu;
    for (uint16_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 1u) {
                crc = (crc >> 1u) ^ 0xA001u;
            } else {
                crc >>= 1u;
            }
        }
    }
    return crc;
}

int CheckCrc(const FramePacket *frame)
{
    uint16_t received;
    uint16_t calculated = CalculateCrc((const uint8_t *)frame, frame->length - sizeof(frame->crc16) - sizeof(frame->tail));
    memcpy(&received, (uint8_t *)frame + frame->length - sizeof(frame->crc16) - sizeof(frame->tail), sizeof(frame->crc16));
    return calculated == received;
}

/* ========================================================================== */
/* Frame packet make                                                          */
/* ========================================================================== */

void MakeFrameFixedField(FramePacket *packet, uint8_t dataType, uint8_t funcCode, uint8_t cmdCode)
{
    memset(packet->header, PROTOCOL_HEADER, sizeof(packet->header)); /* 0xDD 0xDD */
    packet->dataType     = dataType;
    packet->functionCode = funcCode;
    packet->commandCode  = cmdCode;
}

void MakeFrameVarField(FramePacket *packet, uint8_t *data, uint16_t dataLength)
{
	packet->length = PROTOCOL_FIXED_LENGTH + dataLength;
	memcpy(packet->data, data, dataLength);
	uint16_t crc16 = CalculateCrc((const uint8_t *)packet, PROTOCOL_FIXED_LENGTH + dataLength - sizeof(packet->crc16) - sizeof(packet->tail));
    memcpy(packet->data + dataLength, &crc16, sizeof(crc16));
    memset(packet->data + dataLength + sizeof(crc16), PROTOCOL_TAIL, sizeof(packet->tail)); /* 0xFF 0xFF */
}

/* ========================================================================== */
/* Frame packet send                                                          */
/* ========================================================================== */

void SendInitPowerOnFrame(int uartNum, uint8_t dataType, uint8_t falutCode)
{
    FramePacket frame;
    MakeFrameFixedField(&frame, dataType, FUNC_CODE_SEND, INIT_SUCCESS_SIGNAL);
    MakeFrameVarField(&frame, &falutCode, 1u); /* 1 = falutCode */
    UartSend(uartNum, (uint8_t *)&frame, frame.length);
}

void SendCardStatusFrame(int uartNum, uint8_t dataType, uint8_t cardStatus)
{
    FramePacket frame;
    MakeFrameFixedField(&frame, dataType, FUNC_CODE_SEND, CMD_NONE);
    MakeFrameVarField(&frame, &cardStatus, 1u); /* 1 = cardStatus */
    UartSend(uartNum, (uint8_t *)&frame, frame.length);
}

void SendDetectionDataFrame(int uartNum, uint8_t dataType, ChannelInfo *channelInfo, uint8_t channelNum)
{
    FramePacket frame;
    MakeFrameFixedField(&frame, dataType, FUNC_CODE_SEND, CMD_NONE);
    MakeFrameVarField(&frame, (uint8_t *)channelInfo, sizeof(ChannelInfo) * channelNum);
    UartSend(uartNum, (uint8_t *)&frame, frame.length);
}

void SendThresholdModifyResultFrame(int uartNum, uint8_t dataType, uint8_t thresholdModifyStatus)
{
    FramePacket frame;
    MakeFrameFixedField(&frame, dataType, FUNC_CODE_SEND, CMD_NONE);
    MakeFrameVarField(&frame, &thresholdModifyStatus, 1u); /* 1 = thresholdModifyStatus */
    UartSend(uartNum, (uint8_t *)&frame, frame.length);
}

/* ========================================================================== */
/* Frame packet receive handle                                                */
/* ========================================================================== */

/**
 * @brief Parse the start-detection frame.
 *
 * @param[in]  recvData  Received complete frame data.
 *
 * @return Number of project numbers parsed; negative on error.
 */
int HandleStartDetectionFrame(SerialData *recvData)
{
    FramePacket *frame = (FramePacket *)recvData->data;
    uint16_t frameLen = frame->length;
    memcpy((uint8_t *)frame, recvData->data, frameLen);

    if (frame->header[0] != PROTOCOL_HEADER || frame->header[1] != PROTOCOL_HEADER) {
        return -1;
    }
    if (!CheckCrc(frame)) {
        return -2;
    }
    if ((frame->length - PROTOCOL_FIXED_LENGTH) % PRJNUM_FIXED_SIZE) {
        return -3;
    }
    int count = (frame->length - PROTOCOL_FIXED_LENGTH) / PRJNUM_FIXED_SIZE;
    if (count > 0) {

    	ChannelInfo *channelInfo = malloc(sizeof(ChannelInfo) * count);
		if (channelInfo == NULL) {
			return -5;
		}

		/* TODO */
		// to do data processing and acquisition

		/* debug */
		ChannelInfo chInfo[10] = {
			{ .result = RESULT_NEGATIVE, .probability = PROB_LOW },          /* A0 */
			{ .result = RESULT_POSITIVE, .probability = PROB_HIGH },         /* B10 */
			{ .result = RESULT_INVALID,  .probability = PROB_MEDIUM },       /* C30 */
			{ .result = RESULT_NEGATIVE, .probability = PROB_NONE },         /* D39 */
			{ .result = RESULT_POSITIVE, .probability = PROB_LOW },          /* E50 */
			{ .result = RESULT_INVALID,  .probability = PROB_HIGH },         /* F63 */
			{ .result = RESULT_NEGATIVE, .probability = PROB_MEDIUM },       /* G20 */
			{ .result = RESULT_POSITIVE, .probability = PROB_LOW },          /* H8  */
			{ .result = RESULT_INVALID,  .probability = PROB_HIGH },         /* I69 */
			{ .result = RESULT_NEGATIVE, .probability = PROB_MEDIUM },       /* J99 */
		};
		/* debug */
		for (int i = 0; i < count; i++) {
			memcpy(channelInfo + i, frame->data + i * PRJNUM_FIXED_SIZE, PRJNUM_FIXED_SIZE);
			memcpy(&(channelInfo + i)->result, &(chInfo + i)->result, sizeof(channelInfo->result));
			memcpy(&(channelInfo + i)->probability, &(chInfo + i)->probability, sizeof(channelInfo->probability));
		}
		SendDetectionDataFrame(UART_CUSTOM, frame->dataType, channelInfo, count);

		free(channelInfo);
	}
    return count;
}

/**
 * @brief Parse the set-threshold frame.
 *
 * @param[in]  recvData       Received complete frame data.
 *
 * @return return Number of project numbers parsed; negative on error.
 */
int HandleSetThresholdFrame(SerialData *recvData)
{
	ThresholdInfo *thresholdInfo = NULL;
    const FramePacket *frame = (const FramePacket *)recvData->data;
    uint16_t frameLen = frame->length;
    memcpy((uint8_t *)frame, recvData->data, frameLen);

    if (frame->header[0] != PROTOCOL_HEADER || frame->header[1] != PROTOCOL_HEADER) {
        return -1;
    }
    if (!CheckCrc(frame)) {
        return -2;
    }
    if ((frame->length - PROTOCOL_FIXED_LENGTH) % (PRJNUM_FIXED_SIZE + sizeof(thresholdInfo->threshold))) {
        return -3;
    }
    int count = (frame->length - PROTOCOL_FIXED_LENGTH) / (PRJNUM_FIXED_SIZE + sizeof(thresholdInfo->threshold));
    if (count > 0) {
		thresholdInfo = (ThresholdInfo *)malloc(sizeof(ThresholdInfo) * MAX_CHANNELS);
		if (thresholdInfo == NULL) {
			return -4;
		}
		memcpy((uint8_t *)thresholdInfo, frame->data, frame->length - PROTOCOL_FIXED_LENGTH);

		/* TODO */
		// to do data processing and acquisition

		/* debug */
		uint8_t thresholdModifyStatus = THRESHOLD_SET_FAILED;
		/* debug */
		SendThresholdModifyResultFrame(UART_CUSTOM, frame->dataType, thresholdModifyStatus);

		free(thresholdInfo);
    }
    return count;
}

/* ========================================================================== */
/* Freertos task create                                                       */
/* ========================================================================== */

#ifdef MOUDULE_SA_MF210A

void Sa_Mf210a_Task(void *unused)
{
	SerialData receivedData;
	FramePacket *pkt = NULL;
	uint8_t cmdType = 0;
	while (1) {
		if (xQueueReceive(g_queueUart[QUEUE_UART_SA_MF210A].queue, &receivedData, pdMS_TO_TICKS(100))) {
			pkt = (FramePacket *)receivedData.data;
			cmdType = pkt->commandCode;
			if (cmdType > 0 && cmdType < CMD_NUM_MAX) {
				cmdCallBack[cmdType](&receivedData);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(150));
	}
}

#endif
