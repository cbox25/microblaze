/*
 * gpio.h
 *
 *  Created on: 2024.4.13
 *      Author: wwy
 */
#if 0
#ifndef GPIO_H
#define GPIO_H

#include "xgpio.h"

extern XGpio g_gpio;

#define GPIO_DEVICE_ID    XPAR_GPIO_0_DEVICE_ID
#define GPIO_OUTPUT    0
#define GPIO_INPUT    1

extern void InitGpio(XGpio *gpio);
extern void Gmax4002GpioStartSeq(void);

#endif /* GPIO_H */
#endif

