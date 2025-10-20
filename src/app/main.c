#include <operate_reg.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "xparameters.h"
#include "xil_printf.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "xil_io.h"
#include "xtmrctr.h"
#include "xintc.h"
#include "flash.h"
#include "firmware_upgrade.h"
#include "gencp.h"
#include "task.h"
#include "common.h"
#include "timer.h"
#include "gpio.h"
#include "i2c.h"
#include "cmd.h"
#include "sa_mf210a.h"

#ifdef MODULE_UART
void RegReadWriteTask(void *unused)
{
    SerialData receivedData;
    static uint8_t count = 0;
    uint32_t reg = 0x44A209F4;
    while (1) {
        if (xQueueReceive(g_queueUart[QUEUE_UART_REG_OPERATE].queue, &receivedData, pdMS_TO_TICKS(100))) {
            OperateReg(&receivedData);
        }
        if(count++ == 2) {
        	ReadReg(&reg);
        }
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
#else
void RegReadWriteTask(void *unused){}
#endif

#ifdef MODULE_GENCP
void TaskGenCP(void *unused)
{
    int ret = OK;
    SerialData receivedData;
    static uint8_t cnt = 0;

	if (HasXmlInFlash()) {
    	LOG_DEBUG("XML exists in flash, skip writing camera regs\r\n");
    } else {
        LOG_DEBUG("No XML in flash, write camera regs to flash\r\n");
        GencpWriteXmlFromArray();
    }

    while (1) {
        if (xQueueReceive(g_queueUart[QUEUE_UART_GENCP].queue, &receivedData, pdMS_TO_TICKS(10))) {
            ret = ParseGencpRecvPkg(receivedData.uartNum, receivedData.data, receivedData.size);
		#ifdef FUNC_PASSTHROUGH
			if (receivedData.uartNum == UART_0) {
				UartSend(UART_1, receivedData.data, receivedData.size);
			} else if (receivedData.uartNum == UART_1) {
				UartSend(UART_0, receivedData.data, receivedData.size);
			}
		#else
			ret = ParseGencpRecvPkg(receivedData.uartNum, receivedData.data, receivedData.size);
		#endif

            /* TODO: handle with ret */
        }
#if 0
        } else if (xQueueReceive(g_queueUart[QUEUE_UART_GENCP_XML].queue, &receivedData, pdMS_TO_TICKS(10))) {
            ret = ParseGencpXmlRecvPkg(receivedData.uartNum, receivedData.data, receivedData.size);
            /* TODO: handle with ret */
        }
#endif
        if (((cnt ++) % 100) == 0) {
        	GencpGetPeriodData();
        	cnt = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif

#if 0
void TestFlash(void)
{
    uint32_t blockStartAddr = FLASH_SPACE_FIRMWARE_UPDATE_START_ADDR;
    int8_t spiId = SPI_DEV_ID_ARG;
    int blkCnt = 0, i = 0;
    uint8_t testBuf[128], readBuf[128];

    memset(readBuf, 0x00, sizeof(readBuf));

    for (i = 0; i < sizeof(testBuf); i ++) {
        testBuf[i] = i;
    }

    /* erase block */
    NORFLASH_ERASE_4K_SECTOR(spiId, blockStartAddr);
    DelayMs(3);

    blkCnt = (sizeof(testBuf) + NOR_FLASH_RW_MAX_SIZE - 1) / NOR_FLASH_RW_MAX_SIZE;
    /* write block */
    for (i = 0; i < blkCnt; i ++) {
        NorFlashWriteBuf(spiId,
                        blockStartAddr + i * NOR_FLASH_RW_MAX_SIZE,
                        NOR_FLASH_RW_MAX_SIZE,
                        testBuf + i * NOR_FLASH_RW_MAX_SIZE);
        DelayMs(2);
    }

    /* read block */
    for (i = 0; i < blkCnt; i ++) {
        NorFlashReadBuf(spiId,
                        blockStartAddr + i * NOR_FLASH_RW_MAX_SIZE,
                        NOR_FLASH_RW_MAX_SIZE,
                        readBuf);
        DelayMs(2);

        PrintBuf(readBuf, sizeof(readBuf), "readBuf");
    }

    LOG_DEBUG("--end--\r\n");
}
#endif

#ifdef MODULE_SPI_FLASH
void InitRegArgs(void)
{
    /* read args mode */
    NorFlashReadBuf(SPI_DEV_ID_ARG, FLASH_SPACE_REG_ARG_MODE_ADDR, sizeof(g_globalParams), &g_globalParams);

    if (g_globalParams.argMode >= ARG_MODE_MAX) {
    	g_globalParams.argMode = 0;
    }

    /* read args from flash and write args to regs */
    LoadArgsFromFlash(g_globalParams.argMode);
}
#else
void InitRegArgs(void){}
#endif

#ifdef MODULE_UART
void UartSendVersion(void)
{
    uint8_t buf[32] = {0};
    strcat((char *)buf, BOARD);
    strcat((char *)buf, VERSION);
    UartSend(UART_0, buf, (int)strlen(buf));
}
#else
void UartSendVersion(void){}
#endif

#ifdef MODULE_MIPI
#define MIPI_BASEADDR XPAR_MIPI_CSI2_TX_SUBSYST_0_BASEADDR

void InitMipi(void)
{
    Reg reg;

    /* write offset 0x00 */
    reg.addr = MIPI_BASEADDR | 0x00;
    ReadReg(&reg);
    reg.value &= 0x7;
    reg.value |= 0x3; /* soft reset and enable*/
    WriteReg(&reg);
    reg.value &= 0xfffffffd; /* set soft reset as 0*/
    WriteReg(&reg);

    /* write offset 0x04 */
    reg.addr = MIPI_BASEADDR | 0x04;
    ReadReg(&reg);
    reg.value &= 0x601B;
    WriteReg(&reg);

    /* wait ready signal */
    reg.addr = MIPI_BASEADDR | 0x00;
    Reg axiReg;
    axiReg.addr = AXI_BASE_ADDR;
    axiReg.value = 1;
    while (1) {
        ReadReg(&reg);
        if ((reg.value & 0x4) == 0x4) {
            WriteReg(&axiReg);
            break;
        }
    }

    LOG_DEBUG("init mipi reg OK\r\n");
}
#else
void InitMipi(void){}
#endif

#ifdef MODULE_UART
void SendVersion(void)
{
	char buf[32] = {0};

	/* FPGA version */
	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, FPGA_VERSION_START_ADDR, FPGA_VERSION_LEN);
	LOG_DEBUG("fpga version: %s\r\n", buf);
	UartSend(UART_0, buf, FPGA_VERSION_LEN);

	/* microblaze version */
	memset(buf, 0x00, sizeof(buf));
	strcat(buf, BOARD);
	strcat(buf, VERSION);
	UartSend(UART_0, buf, (int)strlen(buf));
}
#endif

#if 0
void FreertosInitTask(void *unused)
{
    XIntc interruptController;

    XIntc_Initialize(&interruptController, XPAR_INTC_0_DEVICE_ID);

#ifdef MODULE_UART
#if UART_NUM_MAX == 1
    int uartIrqIntr[UART_NUM_MAX] = {
            XPAR_AXI_INTC_UART_CMD_IRQ_INTR
    };
#elif UART_NUM_MAX == 2
    int uartIrqIntr[UART_NUM_MAX] = {
            XPAR_AXI_INTC_UART_0_IRQ_INTR ,
            XPAR_AXI_INTC_UART_1_IRQ_INTR
    };

    /* uart */
    for (int i = 0; i < UART_NUM_MAX; i ++) {
        UartInit(i, &g_uart[i], &g_uartConfig[i], BAUD_115200_VALUE);
        XIntc_Connect(&interruptController, uartIrqIntr[i], (XInterruptHandler)g_uartIrqCb[i], &g_uart[i]);
        XIntc_Enable(&interruptController, uartIrqIntr[i]);
    }
#endif
#endif /* MODULE_UART */

#ifdef MODULE_SPI_FLASH
    /* spi */
    for (int i = 0; i < SPI_DEV_NUM_MAX; i ++) {
        InitXspi(i);
        DelayMs(10);
    }

    ClearUpgradeStatus(&g_upgrade);
#endif /* MODULE_SPI_FLASH */

    /* timer */
    XTmrCtr_Initialize(&g_timerCtrl, XPAR_AXI_TIMER_DEVICE_ID);
    XTmrCtr_SetResetValue(&g_timerCtrl, 0, TIMER_LOAD_VALUE);
    XTmrCtr_SetOptions(&g_timerCtrl, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION);
    XIntc_Connect(&interruptController, XPAR_AXI_INTC_TIMER_IRQ_INTR, (XInterruptHandler)vTickISR, &g_timerCtrl);
    XTmrCtr_Start(&g_timerCtrl, 0);

    XIntc_Enable(&interruptController, XPAR_AXI_INTC_TIMER_IRQ_INTR);
    XIntc_Start(&interruptController, XIN_REAL_MODE);
    Xil_ExceptionEnable();

    // InitAlgrithmParam();
    // CheckAlgrithmParam();

#ifdef MODULE_GENCP /* TODO */
    InitGencp();
#endif

    /* test flash */
    // TestFlash();
    UartSendVersion();
    InitRegArgs();

    xil_printf("init task OK\r\n");
    vTaskDelete(NULL);
}

#else

void FreertosInitTask(void *unused)
{
    XIntc intCtrl;
    int i = 0;
    int uartIrqIntr[UART_NUM_MAX] = {
            XPAR_AXI_INTC_UART_CMD_IRQ_INTR
#ifdef MODULE_UART_1
            , XPAR_AXI_INTC_UART_UPDATE_IRQ_INTR
#endif
    };

    /* spi init device */
    for (i = 0; i < SPI_DEV_NUM_MAX; i ++) {
        InitXspi(i);
        DelayMs(10);
    }

    /* uart init device */
    for (i = 0; i < UART_NUM_MAX; i ++) {
#if defined(LINESCAN)
        UartInit(i, &g_uart[i], &g_uartConfig[i], BAUD_230400_VALUE);
#else
        UartInit(i, &g_uart[i], &g_uartConfig[i], BAUD_115200_VALUE);
#endif
    }

#ifdef MODULE_I2C
    /* i2c init device */
    InitI2c(&g_i2cDev);
#endif

    /* timer init object */
    XTmrCtr_Initialize(&g_timerCtrl, XPAR_AXI_TIMER_DEVICE_ID);
    XTmrCtr_SetResetValue(&g_timerCtrl, 0, TIMER_LOAD_VALUE);
    XTmrCtr_SetOptions(&g_timerCtrl, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION);

    XIntc_Initialize(&intCtrl, XPAR_INTC_0_DEVICE_ID);

    /* uart interrupt connect */
    for (i = 0; i < UART_NUM_MAX; i ++) {
        XIntc_Connect(&intCtrl, uartIrqIntr[i], (XInterruptHandler)g_uartIrqCb[i], &g_uart[i]);
    }

#ifdef MODULE_I2C
    /* i2c interrupt connect */
	XIntc_Connect(&intCtrl, AXI_INTC_IIC_IRQ_VEC_ID, (XInterruptHandler)XIic_InterruptHandler, &g_i2cDev);
#endif

    /* timer interrupt connect */
    XIntc_Connect(&intCtrl, XPAR_AXI_INTC_TIMER_IRQ_INTR, (XInterruptHandler)vTickISR, &g_timerCtrl);

    /* uart intc enable */
    for (i = 0; i < UART_NUM_MAX; i ++) {
        XIntc_Enable(&intCtrl, uartIrqIntr[i]);
    }
#ifdef MODULE_I2C
    /* i2c intc enable */
	XIntc_Enable(&intCtrl, AXI_INTC_IIC_IRQ_VEC_ID);
#endif
    /* timer intc enable */
    XIntc_Enable(&intCtrl, XPAR_AXI_INTC_TIMER_IRQ_INTR);

    XIntc_Start(&intCtrl, XIN_REAL_MODE);
    Xil_ExceptionEnable();

    XTmrCtr_Start(&g_timerCtrl, 0);

#ifdef MODULE_I2C
	InitI2cSlaveIntrHandler(&intCtrl);
    XIic_Start(&g_i2cDev); /* i2c initial status: 0xc0 */
#endif

    ClearUpgradeStatus(&g_upgrade);

    InitWriteCameraRegListToFlash();
    InitRegArgs();

#ifdef MODULE_GENCP /* TODO */
    InitGencp();
#endif

    /* test flash */
    // TestFlash();
#ifdef MODULE_UART
    SendVersion();
#endif

#ifdef MODULE_SA_MF210A
    /* TODO */
    // to do Power-on initialization

    /* debug */
    DelayMs(1000);
    uint8_t dataType = 0x00;
    Reg device_status = {
    	.addr = 0x00000A24,
    	.value = 0x01,
    	.delay = 10
    };
    Reg card_status = {
		.addr = 0x00000A28,
		.value = 0x01,
		.delay = 10
	};
    WriteReg(&device_status);
    WriteReg(&card_status);
    Reg rx1 = {
		.addr = 0x00000A24,
		.value = 0x00,
		.delay = 10
	};
	Reg rx2 = {
		.addr = 0x00000A28,
		.value = 0x00,
		.delay = 10
	};
	ReadReg(&rx1);
	ReadReg(&rx2);

	uint8_t falutCode = DEVICE_STATUS_PENDING;
	falutCode = rx1.value;
    SendInitPowerOnFrame(UART_CUSTOM, dataType, falutCode);
    DelayMs(1000);
	uint8_t cardStatus = STATUS_CARD_ABSENT;
	cardStatus = rx2.value;
    SendCardStatusFrame(UART_CUSTOM, dataType, cardStatus);
    /* debug */

#endif

#ifdef ISP_FFC
    InitFfc();
#endif
#ifdef ISP_DPC
    InitDpc();
#endif

    xil_printf("init task OK\r\n");

    vTaskDelete(NULL);
}
#endif

void UnusedTask(void *unused)
{
    while (1) {
        LOG_DEBUG("%s\r\n", __FUNCTION__);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
void CreateFreertosTask(void)
{
    xTaskCreate(FreertosInitTask, "Init", configMINIMAL_STACK_SIZE * 10, NULL, configMAX_PRIORITIES - 1, NULL);
#if !defined(LINESCAN)
#ifdef MODULE_SPI_FLASH
    xTaskCreate(FirmwareUpdateTask, "Update", configMINIMAL_STACK_SIZE * 8, NULL, configMAX_PRIORITIES - 3, NULL);
#endif

#ifdef MODULE_UART
    xTaskCreate(RegReadWriteTask, "RegOp", configMINIMAL_STACK_SIZE * 8, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(CmdTask, "Cmd", configMINIMAL_STACK_SIZE * 4, NULL, configMAX_PRIORITIES - 3, NULL);
#endif
#endif

#ifdef MODULE_SA_MF210A
    xTaskCreate(Sa_Mf210a_Task, "sa_mf210a", configMINIMAL_STACK_SIZE * 4, NULL, configMAX_PRIORITIES - 3, NULL);
#endif

#ifdef MODULE_GENCP
    xTaskCreate(TaskGenCP, "TaskGenCP", configMINIMAL_STACK_SIZE * 10, NULL, configMAX_PRIORITIES - 2, NULL);
#endif

#if !defined(MODULE_SPI_FLASH) && !defined(MODULE_UART) && !defined(MODULE_GENCP)
    xTaskCreate(UnusedTask, "Unused", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 4, NULL);
#endif
}

#ifdef MODULE_UART
void CreateQueues(void)
{
    for (int i = 0; i < QUEUE_UART_NUM; i++) {
        g_queueUart[i].queue = xQueueCreate(g_queueUart[i].depth, g_queueUart[i].dataLen);
        if (g_queueUart[i].queue == NULL) {
            LOG_DEBUG("create queues failed\r\n");
            return;
        }
    }
}
#else
void CreateQueues(void){}
#endif

int main(void)
{
    xil_printf("main start\r\n");

    DelayMs(1000); /* wait for FPGA */

    CreateQueues();
    CreateFreertosTask();
    vTaskStartScheduler();

    while (1) {
        ;
    }

    return 0;
}
