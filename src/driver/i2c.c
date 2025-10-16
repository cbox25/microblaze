/*
 * i2c.c
 *
 *  Created on: 2025.8.4
 *      Author: wwy
 */


#include "config.h"

#ifdef MODULE_I2C

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "xiic.h"
#include "xiic_l.h"
#include "gpio.h"
#include "xparameters.h"
#include "xintc.h"
#include "xil_exception.h"
#include "i2c.h"
#include "common.h"


XIic g_i2cDev;
static I2cStatus g_i2cStatus;
uint8_t g_i2cSlaveRxBuf[I2C_BUF_MAX_LEN];
uint8_t g_i2cSlaveTxBuf[I2C_BUF_MAX_LEN];

int I2cMasterWriteBuf(XIic *i2c, uint16_t regAddr, uint8_t *buf, uint32_t len)
{
	uint8_t send_buf[I2C_DATA_NUM] = { 0 };
	uint32_t sendCnt = 0;

	if (i2c == NULL || buf == NULL || len == 0)
		return ERROR;

	regAddr = ConvertEndian16(regAddr);
	memset(send_buf, 0x00, sizeof(send_buf));
	memcpy(send_buf, &regAddr, sizeof(regAddr));
	memcpy(send_buf + sizeof(regAddr), buf, len);

	/* write devAddr, regAddr, buf */
	taskENTER_CRITICAL();
	sendCnt = XIic_Send(i2c->BaseAddress, i2c->AddrOfSlave, send_buf, sizeof(regAddr) + len, XIIC_STOP);
	if (sendCnt != sizeof(regAddr) + len) {
		taskEXIT_CRITICAL();
		return ERROR - 1;
	}
	taskEXIT_CRITICAL();

	return OK;
}

int I2cMasterReadBuf(XIic *i2c, uint16_t regAddr, uint8_t *buf, uint32_t len)
{
	uint8_t send_buf[2] = { 0 };
	uint32_t sendCnt = 0;
	uint32_t recvCnt = 0;

	if (i2c == NULL || buf == NULL || len == 0)
		return ERROR;

	regAddr = ConvertEndian16(regAddr);
	memcpy(send_buf, &regAddr, sizeof(regAddr));

	/* write devAddr, regAddr */
	taskENTER_CRITICAL();
	sendCnt = XIic_Send(i2c->BaseAddress, i2c->AddrOfSlave, send_buf, sizeof(regAddr), XIIC_STOP);
	if (sendCnt != sizeof(regAddr)) {
		taskEXIT_CRITICAL();
		return ERROR - 2;
	}
	taskEXIT_CRITICAL();

	/* write devAddr of Read, read buf */
	taskENTER_CRITICAL();
	recvCnt = XIic_Recv(i2c->BaseAddress, i2c->AddrOfSlave, buf, len, XIIC_STOP);
	if (recvCnt != len) {
		taskEXIT_CRITICAL();
		return ERROR - 3;
	}
	taskEXIT_CRITICAL();

	return OK;
}

int InitI2c(XIic *i2c)
{
	int status = 0;

	/* init i2c */
	status = XIic_Initialize(i2c, I2C_DEV_ID);
	if (status != XST_SUCCESS)
		return ERROR;

	/* set i2c address */
	status = XIic_SetAddress(i2c, XII_ADDR_TO_RESPOND_TYPE, I2C_ADDR);
	if (status != XST_SUCCESS)
		return ERROR;

	return OK;
}

int I2cSlaveReadData(uint8_t *buf, uint16_t len)
{
	int status;

	/* Set the defaults. */
	g_i2cStatus.rxComplete = 1;

	/* Start the IIC device. */
	status = XIic_Start(&g_i2cDev);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set the Global Interrupt Enable. */
	XIic_IntrGlobalEnable(g_i2cDev.BaseAddress);

	/* Wait for AAS interrupt and completion of data reception. */
	while ((g_i2cStatus.rxComplete) || (XIic_IsIicBusy(&g_i2cDev) == TRUE)) {
		if (g_i2cStatus.slaveRead) {
			XIic_SlaveRecv(&g_i2cDev, buf, len);
			g_i2cStatus.slaveRead = 0;
		}
	}

	/* Disable the Global Interrupt Enable. */
	XIic_IntrGlobalDisable(g_i2cDev.BaseAddress);

	/* Stop the IIC device. */
	status = XIic_Stop(&g_i2cDev);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int I2cSlaveWriteData(uint8_t *buf, uint16_t len)
{
	int status;

	/* Set the defaults. */
	g_i2cStatus.txComplete = 1;

	/* Start the IIC device. */
	status = XIic_Start(&g_i2cDev);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set the Global Interrupt Enable. */
	XIic_IntrGlobalEnable(g_i2cDev.BaseAddress);

	/* Wait for AAS interrupt and transmission to complete. */
	while ((g_i2cStatus.txComplete) || (XIic_IsIicBusy(&g_i2cDev) == TRUE)) {
		if (g_i2cStatus.slaveWrite) {
			XIic_SlaveSend(&g_i2cDev, buf, len);
			g_i2cStatus.slaveWrite = 0;
		}
	}

	/* Disable the Global Interrupt Enable bit. */
	XIic_IntrGlobalDisable(g_i2cDev.BaseAddress);

	/* Stop the IIC device. */
	status = XIic_Stop(&g_i2cDev);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int I2cSlaveCmd(void)
{
	static uint8_t Index;
	/* The IIC Master on this bus should initiate the transfer
	 * and write data to the slave at this instance. */
	I2cSlaveReadData(g_i2cSlaveRxBuf, sizeof(g_i2cSlaveRxBuf));
	PrintBuf(g_i2cSlaveRxBuf, sizeof(g_i2cSlaveRxBuf), "i2c example read buf");

#if 0
	for (Index = 0; Index < 128; Index++) {
		g_i2cSlaveTxBuf[Index] = Index;
	}
	/* The IIC Master on this bus should initiate the transfer and read data from the slave. */
	I2cSlaveWriteData(128);
#endif

	return XST_SUCCESS;
}

static void I2cStatusHandler(XIic *i2cDev, int event)
{
	/* Check whether the event is to write or read the data from the slave. */
	if (event == XII_MASTER_WRITE_EVENT) {
		g_i2cStatus.slaveRead = 1; /* Its a Write request from Master. */
	} else {
		g_i2cStatus.slaveWrite = 1; /* Its a Read request from the master. */
	}
}

static void I2cSendHandler(XIic *i2cDev)
{
	g_i2cStatus.txComplete = 0;
}

static void I2cReceiveHandler(XIic *i2cDev)
{
	g_i2cStatus.rxComplete = 0;
}

void InitI2cSlaveIntrHandler(XIntc *intCtrl)
{
	/* Include the Slave functions. */
	XIic_SlaveInclude();

	/* Set the Transmit, Receive and status Handlers. */
	XIic_SetStatusHandler(&g_i2cDev, &g_i2cDev, (XIic_StatusHandler) I2cStatusHandler);
	XIic_SetSendHandler(&g_i2cDev, &g_i2cDev, (XIic_Handler) I2cSendHandler);
	XIic_SetRecvHandler(&g_i2cDev, &g_i2cDev, (XIic_Handler) I2cReceiveHandler);
}

#endif /* MODULE_I2C */
