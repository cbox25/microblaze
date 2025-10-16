/*
 * i2c.h
 *
 *  Created on: 2025.8.4
 *      Author: wwy
 */
#ifndef I2C_H
#define I2C_H

#include "config.h"
#ifdef MODULE_I2C

#include <stdint.h>
#include "xiic.h"

#define I2C_DEV_ID                     XPAR_IIC_0_DEVICE_ID
#define AXI_INTC_IIC_IRQ_VEC_ID        XPAR_INTC_0_IIC_0_VEC_ID

#define I2C_ADDR            0x10
#define I2C_DATA_NUM		8
#define I2C_BUF_MAX_LEN		512

typedef struct {
	volatile uint8_t txComplete;
	volatile uint8_t rxComplete;
	volatile uint8_t slaveRead;
	volatile uint8_t slaveWrite;
} I2cStatus;

extern XIic g_i2cDev;
extern uint8_t g_i2cSlaveRxBuf[I2C_BUF_MAX_LEN];
extern uint8_t g_i2cSlaveTxBuf[I2C_BUF_MAX_LEN];

int InitI2c(XIic *i2cGmax4002);
int I2cMasterWriteBuf(XIic *i2c, uint16_t regAddr, uint8_t *buf, uint32_t len);
int I2cMasterReadBuf(XIic *i2c, uint16_t regAddr, uint8_t *buf, uint32_t len);
void InitI2cSlaveIntrHandler(XIntc *intCtrl);
int I2cSlaveCmd(void);
int I2cSlaveReadData(uint8_t *buf, uint16_t len);
int I2cSlaveWriteData(uint8_t *buf, uint16_t len);

#endif /* MODULE_I2C */

#endif /* I2C_H */
