#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "uart.h"
#include "xil_io.h"
#include "packet_sr.h"

#ifdef MODULE_GENCP
#include "gencp.h"
#endif

#ifdef MODULE_UART
Uart g_uart[UART_NUM_MAX];
UartConfig g_uartConfig[UART_NUM_MAX];

#if UART_NUM_MAX == 1
UartIrqCb g_uartIrqCb[UART_NUM_MAX] = {
        Uart0IrqHandler
};
#elif (UART_NUM_MAX == 2)
UartIrqCb g_uartIrqCb[UART_NUM_MAX] = {
        Uart0IrqHandler,
        Uart1IrqHandler
};
#endif

UartQueue g_queueUart[QUEUE_UART_NUM] = {
    /* queue, depth, dateLen */
    {NULL, QUEUE_DEPTH_UPDATE,        QUEUE_LEN_UPDATE},
    {NULL, QUEUE_DEPTH_REG_OP,         QUEUE_LEN_REG_OP},
    {NULL, QUEUE_DEPTH_READ_MEM,     QUEUE_LEN_READ_MEM},
    {NULL, QUEUE_DEPTH_HANDSHAKE,     QUEUE_LEN_HANDSHAKE},
    {NULL, QUEUE_DEPTH_CMD,         QUEUE_LEN_CMD},
	{NULL, QUEUE_DEPTH_SA_MF210A,    QUEUE_LEN_SA_MF210A},
    {NULL, QUEUE_DEPTH_GENCP_XML,     QUEUE_LEN_GENCP_XML},
    {NULL, QUEUE_DEPTH_GENCP,         QUEUE_LEN_GENCP}
};

#if UART_NUM_MAX == 1
UartConfigReg g_uartConfigReg[UART_NUM_MAX] = {
        /* uart0, cmd */
        {UART_CMD_BASE_ADDR, UART_CMD_TX_DATA_REG, UART_CMD_RX_DATA_REG,
         UART_CMD_LB_BAUD_REG, UART_CMD_INTR_ACK, UART_CMD_RX_READY_COUNT, UART_CMD_TIMER_WAIT},
};
#elif (UART_NUM_MAX == 2)
UartConfigReg g_uartConfigReg[UART_NUM_MAX] = {
        /* uart0, cmd */
        {UART_CMD_BASE_ADDR, UART_CMD_TX_DATA_REG, UART_CMD_RX_DATA_REG,
         UART_CMD_LB_BAUD_REG, UART_CMD_INTR_ACK, UART_CMD_RX_READY_COUNT, UART_CMD_TIMER_WAIT},

        /* uart1, update */
        {UART_UPDATE_BASE_ADDR, UART_UPDATE_TX_DATA_REG, UART_UPDATE_RX_DATA_REG,
         UART_UPDATE_LB_BAUD_REG, UART_UPDATE_INTR_ACK, UART_UPDATE_RX_READY_COUNT, UART_UPDATE_TIMER_WAIT}
};
#endif

static void HandleUartRecvData(SerialData *recvData)
{
    uint8_t type = 0;
    BaseType_t taskWoken = pdFALSE;

    if (recvData->size < 3)
        return;
#if defined(LINESCAN) || defined(ENDOSCOPE) || defined(MODULE_USB3)
    uint32_t head = *(uint32_t *)(recvData->data);
#else
    uint16_t head = *(uint16_t *)recvData->data;
#endif
    type = recvData->data[sizeof(head)];

    if (head == ConvertEndian16(PACKET_START_FLAG) && type <= QUEUE_UART_NUM && type > 0) {
        if (xQueueIsQueueFullFromISR(g_queueUart[type - 1].queue) == pdFALSE) {
            xQueueSendFromISR(g_queueUart[type - 1].queue, recvData, &taskWoken);
        }
#ifdef MODULE_SA_MF210A
    } else if (head == PROTOCOL_HEADER) {
		if (xQueueIsQueueFullFromISR(g_queueUart[QUEUE_UART_SA_MF210A].queue) == pdFALSE) {
			xQueueSendFromISR(g_queueUart[QUEUE_UART_SA_MF210A].queue, recvData, &taskWoken);
		}
#endif
#ifdef MODULE_GENCP
    } else if (head == GENCP_PREFIX_PREAMBLE) {
        if (xQueueIsQueueFullFromISR(g_queueUart[QUEUE_UART_GENCP].queue) == pdFALSE) {
            xQueueSendFromISR(g_queueUart[QUEUE_UART_GENCP].queue, recvData, &taskWoken);
        }
#endif
    } else {
        return;
    }

    portYIELD_FROM_ISR(taskWoken);
}

void Uart0IrqHandler(Uart *uart)
{
    SerialData recvData;

    ClearUartInterrupt(UART_0);

    recvData.size = GetUartRecvLen(UART_0);
    recvData.uartNum = UART_0;
    UartRecv(UART_0, recvData.data, recvData.size);

#if 0
    PrintBuf(recvData.data, recvData.size, "uart0 irq");
#endif

    HandleUartRecvData(&recvData);
}

void Uart1IrqHandler(Uart *uart)
{
    SerialData recvData;

    ClearUartInterrupt(UART_1);

    recvData.size = GetUartRecvLen(UART_1);
    recvData.uartNum = UART_1;
    UartRecv(UART_1, recvData.data, recvData.size);

#if 0
    PrintBuf(recvData.data, recvData.size, "uart1 irq");
#endif

    HandleUartRecvData(&recvData);
}

/* Serial Port Configuration Initialization*/
void UartConfigure(int uartNum, UartConfig *config, uint32_t baudrate)
{
    uint32_t br[BAUD_460800_VALUE + 1] = {9600, 115200, 912600, 230400, 460800}; /* the order must be same as MACROs */

    config->regBaseAddr = g_uartConfigReg[uartNum].baseAddr;
    config->baudRate = br[baudrate];
    Xil_Out32(g_uartConfigReg[uartNum].LbBaudReg, baudrate);
    config->useParity = FALSE;
    config->parityOdd = FALSE;
    config->dataBits = 8;
}

/*Empty function*/
static void StubHandler(void *callbackRef, unsigned int byteCount)
{
    (void)callbackRef;
    (void)byteCount;
    Xil_AssertVoidAlways();
}

/* Initialize UART driver statistics*/
void UartClearStats(Uart *uart)
{
    Xil_AssertVoid(uart != NULL);
    Xil_AssertVoid(uart->isReady == XIL_COMPONENT_IS_READY);
    uart->stats.transmitInterrupts = 0;
    uart->stats.receiveInterrupts = 0;
    uart->stats.charactersTransmitted = 0;
    uart->stats.charactersReceived = 0;
    uart->stats.receiveOverrunErrors = 0;
    uart->stats.receiveFramingErrors = 0;
    uart->stats.receiveParityErrors = 0;
}

int UartConfigInit(int uartNum, Uart *uart, UartConfig *config)
{
    /* Assertion to validate parameter validity*/
    Xil_AssertNonvoid(uart != NULL);

    /* Set default values, including setting callback handlers to stubs (i.e., fake or empty handlers)*/
    /* Set the next byte pointer in the send buffer to NULL*/
    uart->sendBuf.nextBytePtr = NULL;
    /*Set the remaining bytes in the send buffer to 0*/
    uart->sendBuf.remainingBytes = 0;
    /*Set the request byte count in the send buffer to 0*/
    uart->sendBuf.requestedBytes = 0;

    /*Receive buffer on the same*/
    uart->recvBuf.nextBytePtr = NULL;
    uart->recvBuf.remainingBytes = 0;
    uart->recvBuf.requestedBytes = 0;

    /*Set instance status to ready*/
    uart->isReady = XIL_COMPONENT_IS_READY;

    /*For cases not using DCR bridges, use the base address directly.*/
    uart->regBaseAddr = g_uartConfigReg[uartNum].baseAddr;


    /*Set the receiver handler to stub*/
    uart->recvHandler = StubHandler;
    uart->sendHandler = StubHandler;

    /*Clear statistics for this driver*/
    UartClearStats(uart);

    return XST_SUCCESS;
}

int UartInit(int uartNum, Uart *uart, UartConfig *uartConfig, uint32_t baudrate)
{
    int status = 0;

    if (uartNum >= UART_NUM_MAX || uart == NULL || uartConfig == NULL || baudrate > BAUD_460800_VALUE)
        return -1;

    UartConfigure(uartNum, uartConfig, baudrate);
    status = UartConfigInit(uartNum, uart, uartConfig);
    if (status != XST_SUCCESS) {
        return status;
    }
    return status;
}

/* Read data from the RX_DATA register of the UART*/
uint8_t UartRecvByte(int uartNum)
{
    return Xil_In8(g_uartConfigReg[uartNum].rxDataReg);
}

void UartSendByte(int uartNum, uint8_t value)
{
    Xil_Out8(g_uartConfigReg[uartNum].txDataReg, value);
}

/* Send data from the soft core to the UART serial port. Upon receiving the message in the TX_DATA register, the UART automatically transmits it to the external device connected to the serial port.*/
void UartSend(int uartNum, uint8_t *data, int length)
{
    for (int i = 0; i < length; i++) {
        Xil_Out8(g_uartConfigReg[uartNum].txDataReg, data[i]);
    }
}

/* Receive a set of data */
void UartRecv(int uartNum, uint8_t *data, int length)
{
    for (int i = 0;i < length; i++) {
        data[i] = UartRecvByte(uartNum);
    }
}

void ClearUartInterrupt(int uartNum)
{
    Xil_Out32(g_uartConfigReg[uartNum].intrAck, UART_CLEAR_ACK);
}

uint32_t GetUartRecvLen(int uartNum)
{
    return Xil_In32(g_uartConfigReg[uartNum].rxReadyCnt);
}

void ClearUart(int uartNum)
{
    uint8_t k = 0;
    uint32_t length = GetUartRecvLen(uartNum);
    for (int i = 0;i < length; i++) {
        k = UartRecvByte(uartNum);
    }
}
#endif /* MODULE_UART */
