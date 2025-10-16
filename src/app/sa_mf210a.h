#ifndef SA_MF210A_H
#define SA_MF210A_H

#include <stdint.h>
#include "config.h"
#include "packet_sr.h"
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* Protocol constants                                                         */
/* -------------------------------------------------------------------------- */
/* Protocol header and tail */
#define PROTOCOL_HEADER     0xDDu
#define PROTOCOL_TAIL       0xFFu

/* Protocol fixed length */
#define PROTOCOL_FIXED_LENGTH  11u
/* fixed length = header(2) + length(2) + dataType(1) + functionCode(1) + commandCode(1) + crc(2) + tail(2) */
#define PRJNUM_FIXED_SIZE        2u
#define FRAME_DATA_MAX_LENGTH    256u   /* adjust as needed */
#define FRAME_PACKET_MAX_LENGTH  267u	/* adjust as needed */
#define MAX_CHANNELS             20u    /* adjust as needed */

/* Function codes */
#define FUNC_CODE_SEND           0xA1u
#define FUNC_CODE_RECEIVE        0xA2u

/* Command codes */
#define INIT_SUCCESS_SIGNAL      0x01u /* INIT SUCCESS SIGNAL as a special command code */

enum {
    CMD_NONE            = 0x00,  /* No command */
    CMD_START_DETECTION = 0x01,  /* Start detection cycle */
    CMD_SET_THRESHOLD   = 0x02,  /* Set detection threshold */
	CMD_NUM_MAX
};

/* Fault codes */
enum {
    DEVICE_STATUS_NO_FAULT = 0x00,  /* Normal operation */
    DEVICE_STATUS_FAULT    = 0x01,  /* Fault detected */
    DEVICE_STATUS_PENDING  = 0x02   /* Pending */
};

/* Card status */
enum {
    STATUS_CARD_PRESENT = 0x01, /* exist */
    STATUS_CARD_ABSENT  = 0x02  /* not exist */
};

/* Detection results */
enum {
    RESULT_NEGATIVE = 0x01,   /* Negative */
    RESULT_POSITIVE = 0x02,   /* Positive */
    RESULT_INVALID  = 0x03    /* Invalid */
};

/* Probability levels */
enum {
    PROB_NONE   = 0x00,   /* None */
    PROB_LOW    = 0x01,   /* Low */
    PROB_MEDIUM = 0x02,   /* Medium */
    PROB_HIGH   = 0x03    /* High */
};

/* Threshold set status */
enum {
    THRESHOLD_SET_SUCCESS = 0xA0, /* set OK   */
    THRESHOLD_SET_FAILED  = 0xA1  /* set fail */
};

/* -------------------------------------------------------------------------- */
/* Data types                                                                 */
/* -------------------------------------------------------------------------- */
typedef struct __attribute__((packed)) {
    uint8_t prjNum[PRJNUM_FIXED_SIZE]; /* A-Z -> 0x41-0x5A */ /* 0-99 -> 0x00-0x63 */
    uint8_t result;
    uint8_t probability;
} ChannelInfo;

typedef struct __attribute__((packed)) {
    uint8_t prjNum[PRJNUM_FIXED_SIZE]; /* A-Z -> 0x41-0x5A */ /* 0-99 -> 0x00-0x63 */
    uint16_t threshold; /* 0-9999 -> 0x0000-0x270F */
} ThresholdInfo;

typedef struct __attribute__((packed)) {
    uint8_t  header[2];     /* 0xDD 0xDD */
    uint16_t length;        /* total frame length, little-endian */
    uint8_t  dataType;
    uint8_t  functionCode;
    uint8_t  commandCode;
    uint8_t  data[FRAME_DATA_MAX_LENGTH];
    uint16_t crc16;
    uint8_t  tail[2];      /* 0xFF 0xFF */
} FramePacket;

/* ========================================================================== */
/* Frame packet send                                                          */
/* ========================================================================== */
void SendInitPowerOnFrame(int uartNum, uint8_t dataType, uint8_t falutCode);
void SendCardStatusFrame(int uartNum, uint8_t dataType, uint8_t cardStatus);
void SendDetectionDataFrame(int uartNum, uint8_t dataType, ChannelInfo *channelInfo, uint8_t channelNum);
void SendThresholdModifyResultFrame(int uartNum, uint8_t dataType, uint8_t thresholdModifyStatus);

/* ========================================================================== */
/* Frame packet receive handle                                                */
/* ========================================================================== */
int HandleStartDetectionFrame(SerialData *recvData);
int HandleSetThresholdFrame(SerialData *recvData);

/* ========================================================================== */
/* Freertos task create                                                       */
/* ========================================================================== */

#ifdef MOUDULE_SA_MF210A

void Sa_Mf210a_Task(void *unused);

#endif

#endif /* SA_MF210A_H */
