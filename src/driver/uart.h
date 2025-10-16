#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <unistd.h>
#include "xil_types.h"
#include "xstatus.h"
#include "common.h"
#include "sa_mf210a.h"

#ifdef MODULE_UART

typedef void (*UartHandler)(void *callbackRef, unsigned int byteCount);

/* uart0: cmd */
#define XPAR_AXI_INTC_UART_CMD_IRQ_INTR        XPAR_MICROBLAZE_0_AXI_INTC_UART_TOP_0_CARD_TO_HOST_IRQ_INTR
#define UART_CMD_BASE_ADDR       XPAR_UART_TOP_0_BASEADDR /* 0x44A10000 */
#define UART_CMD_TX_DATA_REG     (UART_CMD_BASE_ADDR + 0x050)
#define UART_CMD_RX_DATA_REG     (UART_CMD_BASE_ADDR + 0x04C)
#define UART_CMD_LB_BAUD_REG     (UART_CMD_BASE_ADDR + 0x054)
#define UART_CMD_INTR_ACK        (UART_CMD_BASE_ADDR + 0x044)
#define UART_CMD_RX_READY_COUNT  (UART_CMD_BASE_ADDR + 0x058)
#define UART_CMD_TIMER_WAIT      (UART_CMD_BASE_ADDR + 0x05C)
#define UART_CLEAR_ACK           0x01

#if UART_NUM_MAX == 2
/* uart1, update */
#define XPAR_AXI_INTC_UART_UPDATE_IRQ_INTR        XPAR_U3_MIB_MIB_BD_I_AXI_INTC_0_U3_MIB_MIB_BD_I_UART_TOP_1_CARD_TO_HOST_IRQ_INTR
#define UART_UPDATE_BASE_ADDR       XPAR_U3_MIB_MIB_BD_I_UART_TOP_1_BASEADDR /* 0x44A30000 */
#define UART_UPDATE_TX_DATA_REG     (UART_UPDATE_BASE_ADDR + 0x050)
#define UART_UPDATE_RX_DATA_REG     (UART_UPDATE_BASE_ADDR + 0x04C)
#define UART_UPDATE_LB_BAUD_REG     (UART_UPDATE_BASE_ADDR + 0x054)
#define UART_UPDATE_INTR_ACK        (UART_UPDATE_BASE_ADDR + 0x044)
#define UART_UPDATE_RX_READY_COUNT  (UART_UPDATE_BASE_ADDR + 0x058)
#define UART_UPDATE_TIMER_WAIT      (UART_UPDATE_BASE_ADDR + 0x05C)
#endif

/* uart queue */
#define QUEUE_DEPTH_UPDATE        8
#define QUEUE_DEPTH_REG_OP        4
#define QUEUE_DEPTH_READ_MEM      4
#define QUEUE_DEPTH_HANDSHAKE     2
#define QUEUE_DEPTH_CMD           2
#define QUEUE_DEPTH_SA_MF210A	  2
#define QUEUE_DEPTH_GENCP_XML     4
#define QUEUE_DEPTH_GENCP         4

#define QUEUE_LEN_UPDATE        (PACKET_MAX_LENGTH + sizeof (int)) /* sizeof(SerialData) */
#define QUEUE_LEN_REG_OP        128
#define QUEUE_LEN_READ_MEM      256
#define QUEUE_LEN_HANDSHAKE     32
#define QUEUE_LEN_CMD           (PACKET_MAX_LENGTH + sizeof (int))
#define QUEUE_LEN_SA_MF210A		(FRAME_PACKET_MAX_LENGTH + sizeof(int))
#define QUEUE_LEN_GENCP_XML     (PACKET_MAX_LENGTH + sizeof (int))
#define QUEUE_LEN_GENCP         (PACKET_MAX_LENGTH + sizeof (int))

enum {
    QUEUE_UART_FIRMWARE_UPDATE = 0,
    QUEUE_UART_REG_OPERATE,
    QUEUE_UART_READ_MEM,
    QUEUE_UART_HANDSHAKE,
    QUEUE_UART_CMD,
	QUEUE_UART_SA_MF210A,
    QUEUE_UART_GENCP_XML,
    QUEUE_UART_GENCP, /* must be the last one */
    QUEUE_UART_NUM
};

typedef struct {
    uint32_t baseAddr;
    uint32_t txDataReg;
    uint32_t rxDataReg;
    uint32_t LbBaudReg;
    uint32_t intrAck;
    uint32_t rxReadyCnt;
    uint32_t timerWait;
} UartConfigReg;

typedef enum {
    BAUD_9600_VALUE = 0,
    BAUD_115200_VALUE,
    BAUD_921600_VALUE,
    BAUD_230400_VALUE,
    BAUD_460800_VALUE
} UartBaudRateValue;

typedef struct {
    uint32_t transmitInterrupts;
    uint32_t receiveInterrupts;
    uint32_t charactersTransmitted;
    uint32_t charactersReceived;
    uint32_t receiveOverrunErrors;
    uint32_t receiveParityErrors;
    uint32_t receiveFramingErrors;
} UartStats;

typedef struct {
    uint16_t deviceId;
    UINTPTR regBaseAddr;
    uint32_t baudRate;
    uint8_t  useParity;
    uint8_t  parityOdd;
    uint8_t  dataBits;
} UartConfig;

typedef struct {
    uint8_t *nextBytePtr;
    unsigned int requestedBytes;
    unsigned int remainingBytes;
} UartBuf;

typedef struct {
    UartStats stats;
    UINTPTR regBaseAddr;
    uint32_t isReady;
    UartBuf sendBuf;
    UartBuf recvBuf;
    UartHandler recvHandler;
    void *recvCallBackRef;
    UartHandler sendHandler;
    void *sendCallBackRef;
} Uart;

typedef void (*UartIrqCb)(Uart *uart);

extern Uart g_uart[UART_NUM_MAX];
extern UartConfig g_uartConfig[UART_NUM_MAX];
extern UartQueue g_queueUart[QUEUE_UART_NUM];
extern UartIrqCb g_uartIrqCb[UART_NUM_MAX];

int UartInit(int uartNum, Uart *uart, UartConfig *uartConfig, uint32_t baudrate);
int UartConfigInit(int uartNum, Uart *uart, UartConfig *config);
uint8_t UartRecvByte(int uartNum);
uint32_t GetUartRecvLen(int uartNum);
void UartRecv(int uartNum, uint8_t *data, int length);
void UartSendByte(int uartNum, uint8_t value);
void UartSend(int uartNum, uint8_t *data, int length);
void ClearUartInterrupt(int uartNum);
void ClearUart(int uartNum);

/* irq callback */
void UartCmdIrqHandler(Uart *uart);
void UartUpdateIrqHandler(Uart *uart);

#endif /* MODULE_UART */

#endif /* UART_H */
