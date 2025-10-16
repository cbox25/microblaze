#ifndef COMMON_H
#define COMMON_H

#include "FreeRTOS.h"
#include "timers.h"
#include "xparameters.h"
#include "queue.h"
#include "xil_printf.h"
#include "config.h"

#define __DEBUG

#define AXI_BASE_ADDR         XPAR_M04_AXI_BASEADDR

#define GET_ARRAY_LEN(arr)    (sizeof(arr) / sizeof(arr[0]))

#define PACKET_MAX_LENGTH                   524 /* Maximum length of a file data packet */
#define PACKET_DATA_MAX_LENGTH              512 /* Maximum length of actual data in a file packet */
#define MAX_PACKET_TRANSMISSION_INTERVAL    500000000 /* Maximum packet transmission interval, in nanoseconds */
#define MICROBLAZE_WAIT_TIME                300000000 /* The soft core sends a wait request to the host computer every n seconds when processing data. Unit: nanoseconds */
#define TIMER_LOAD_VALUE                    999999 /* System tick interval */
#define PACKET_START_FLAG                   ((uint16_t) 0xEB90)     /* Packet Start Flag */
#define REPLY_MESSAGE_PACK_DATALEN		    2					    /*The length of the response packet*/
#define FPGA_VERSION_START_ADDR  		    (AXI_BASE_ADDR + 0x0A00)  /*FPGA version storage address*/
#define FPGA_VERSION_LEN		 			16						/*FPGA version length*/

/* PC--->Microblaze */
enum{
	PACKET_TYPE_UPDATE_DATA            = 0x01, /* Data packet type Firmware upgrade package */
	PACKET_TYPE_REG                    = 0x02, /* Packet Type, Register Read/Write */
	PACKET_TYPE_READ_MEMORY            = 0x03, /* Packet Type, Read Memory Data */
	PACKET_TYPE_HANDSHAKE              = 0x04, /* Packet Type, Handshake Packet */
	PACKET_TYPE_CMD                    = 0x05, /* Host computer command response */
	PACKET_TYPE_GENCP_XML              = 0x06, /* WRITE GENCP XML FILE TO FLASH */
};

/* Microblaze--->PC */
enum{
	PACKET_TYPE_UPDATE_REPLY           = 0x80, /* Firmware Upgrade Response Packet */
	PACKET_TYPE_REG_REPLY              = 0x81, /* Register Read/Write Acknowledgment Packet */
	PACKET_TYPE_READ_MEMORY_REPLY      = 0x82, /* Memory Read Response Packet */
	PACKET_TYPE_HANDSHAKE_REPLY        = 0x83, /* Handshake Response Packet */
	PACKET_TYPE_CMD_REPLY              = 0x84, /* cmd */
	PACKET_TYPE_GENCP_XML_REPLY        = 0x85, /* WRITE GENCP XML FILE TO FLASH ACK */
};

/* Microblaze Response to the firmware upgrade data packet */
enum{
	UPDATE_PACKET_ACK                  = 0x01, /* Confirm receipt of the correct firmware upgrade data package. */
	UPDATE_PACKET_ERROR                = 0x02, /* A data packet reception error has occurred; the error code will display the error message. */
	UPDATE_PACKET_WAIT                 = 0x03, /* Soft-core request to host computer to extend wait time */
	UPDATE_PACKET_WRITE_FLASH_ERROR    = 0x04, /* write flash error when update */
};

/* Microblaze Error Code for Responding to Firmware Upgrade Data Packets */
enum{
	UPDATE_PACKET_CRC_ERROR            = 0x01, /* CRC check error */
	UPDATE_PACKET_LENGTH_ERROR         = 0x02, /* Packet Length Error */
};

/* The handshake function field in the handshake packet from PC to Microblaze */
enum{
	HANDSHAKE_CONNECT                  = 0x01, /* The PC requests to establish a connection with the soft core. */
	HANDSHAKE_PC_DISCONNECT            = 0x02, /* PC requests to disconnect from the soft core. */
};

/* Microblaze--->PC Handshake Status Indicator */
enum{
	HANDSHAKE_ACCEPT                   = 0x01, /* Soft-core reception connection */
	HANDSHAKE_MICROBLAZE_DISCONNECT    = 0x02, /* Soft-core request disconnected from PC */
	HANDSHAKE_MICROBLAZE_REJECT        = 0x03, /* Soft-core refuses to connect to PC */
};

/* Reasons for disconnecting or refusing a handshake */
enum{
	HANDSHAKE_MICROBLAZE_BUSY          = 0x01, /* Softcore Busy */
	HANDSHAKE_MICROBLAZE_DENT          = 0x02,/* Soft-core connection prohibited */
	HANDSHAKE_MICROBLAZE_SHAKING       = 0x03, /* The soft core is currently in the handshake state. */
	HANDSHAKE_CRC_ERROR                = 0x04, /* CRC check failed */
	HANDSHAKE_TRANSMISSION_COMPLETE    = 0x05, /* Firmware upgrade transmission complete */
	HANDSHAKE_MICROBLAZE_UNKNOWN_ERROR = 0x06, /* Soft core encountered an unknown error */
	HANDSHAKE_PC_DISCONNECT_ACK        = 0x07, /* The soft core acknowledges the disconnection of the PC request. */
	HANDSHAKE_PC_OUTTIME               = 0x08, /* PC has not sent any packets for an extended period; determine if a timeout has occurred. */
};

/* Define the operation types for register read and write operations */
enum{
	READ_REG  =  0x01, /* Register Read */
	WRITE_REG =  0x02, /* Register Write */
};

/* Define the operational state for register read/write operations */
enum{
	RW_REG_SUCCESS = 0x01, /* Operation successful */
	RW_REG_FALSE   = 0x02, /* Operation failed */
};

/* Define error codes for failed register read/write operations */
enum{
	RW_REG_CRC_ERROE         = 0x01, /* CRC check error */
	RW_REG_LENGTH_ERROR      = 0x02, /* Incorrect length of register read/write data packet */
	WRITE_REG_ADDRESS_IS_BAN = 0x03, /* Register write to non-writable address */
	READ_REG_ADDRESS_IS_BAN  = 0x04, /* Register reads unreadable address */
};

/* General Correct or Incorrect Label */
enum{
	SUCCESS                               =  0, /* successful */
	CRC_FALSE                             =  1, /* CRC check detected data error */
	UART_SEND_FALSE                       =  2, /* Serial port data transmission failed */
	UART_RECEIVE_FALSE                    =  3, /* Serial port data reception failed */
	UART_RECEIVE_timeout                  =  4, /* Serial Port Receive Timeout */
	MICROBLAZE_SEND_PACKET_UNIDENTIFIABLE =  5, /* The data packet received via the serial port cannot be recognized. */
	MICROBLAZE_REFUSE_HANDSHAKE           =  6, /* Soft-core refusal to shake hands */
	HANDSHAKE_FALSE                       =  7, /* Handshake failed */
};

/* Transmission Status Indicator */
enum{
	UN_TRANSMIT       =   0, /* Not transmitted */
	TRANSMITTING      =   1, /* Transferring */
	PC_REQUEST_RESET  =   2, /* Host computer request reset */
	TRANSMIT_FINSH    =   3, /* Transmission complete */
};

/*Handle Csv Status Code*/
enum{
	 HANDLE_CSV_FINISH    	    = 0x01, /*Sending or receiving CSV is completed successfully*/
	 HANDLE_CSV_CRC_ERROR  	    = 0x02, /*CRC issue when processing CSV*/
	 HANDLE_CSV_LENGTH_ERROR    = 0x03, /*Problems with length when processing CSV*/
	 HANDLE_CSV_FILLSIZE_ERROR  = 0x04, /*Problem reading data when processing CSV*/
	 HANDLE_CSV_PACKNUM_ERROR   = 0x05, /*Problem with package count when processing CSV*/
};
/*Handle Version Status Code*/
enum{
	HAND_SEND_VERSION_FINISH = 0x01, /*Send version successfully executed*/
	HAND_SEND_VERSION_ERROR  = 0x02, /*Error in sending version*/
};
/*Regarding the length of information packets in saving and sending CSV*/
enum{
	CSV_SAVEMESSAGE_LENGTH = 0x02, /*Save the length of the CSV information packet*/
	CSV_SENDMESSAGE_LENGTH = 0x03, /*The length of the CSV packet to be sent*/
};

/* General Error Code */
#define OK SUCCESS
#define ERROR (-1)

/* Encapsulate log */

#ifdef __DEBUG
#define LOG_DEBUG(format, ...) do { \
        xil_printf("[%s:%d, %s] "format, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0);
#else
#define LOG_DEBUG(format, ...) ;/*     do {;} while(0);*/
#endif

#define TICKS_TO_MS(t)  ((t) * 1000UL / configTICK_RATE_HZ)

/* queue struct */
typedef struct {
    QueueHandle_t queue;
    int depth;
    int dataLen;
} UartQueue;

typedef struct {
    uint32_t addr;
    uint32_t value;
    uint32_t delay;
} Reg;

uint16_t ConvertEndian16(uint16_t data);
uint32_t ConvertEndian32(uint32_t data);
uint64_t ConvertEndian64(uint64_t data);
void DelayMs(uint32_t ms);
void DelayUs(uint32_t us);
void ReadReg(Reg *reg);
void WriteReg(Reg *reg);
void WriteRegByAddr(uint32_t addr, uint32_t value);
uint32_t ReadRegByAddr(uint32_t addr);
uint8_t U32ToDecStr(uint32_t val, uint8_t *out);

#ifdef ENDOSCOPE
uint32_t ReadRegOah428(uint32_t addr);
uint32_t WriteRegOah428(uint32_t addr, uint32_t value);
#endif
void PrintBuf(uint8_t *buf, int len, const char *name);

#endif
