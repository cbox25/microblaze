/*
 * gpio.c
 *
 *  Created on: 2024.4.13
 *      Author: wwy
 */
#if 0
#include "xgpio.h"
#include "gpio.h"
#include "common.h"

XGpio g_gpio;

#define GPIO_PIN_CTR_EXP_EN            1
#define GPIO_PIN_CTR_START             2
#define GPIO_PIN_SENSOR_RESET_N        4
#define GPIO_PIN_SENSOR_TEXP           8

void InitGpio(XGpio *gpio)
{
    int status = 0;
    uint32_t channel = 0;

    status = XGpio_Initialize(gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        return;
    }

    channel = 1;
    XGpio_SetDataDirection(gpio, channel, GPIO_OUTPUT);
    XGpio_DiscreteClear(gpio, channel, GPIO_PIN_CTR_EXP_EN);
    XGpio_DiscreteClear(gpio, channel, GPIO_PIN_CTR_START);
    XGpio_DiscreteSet(  gpio, channel, GPIO_PIN_SENSOR_RESET_N);
    XGpio_DiscreteClear(gpio, channel, GPIO_PIN_SENSOR_TEXP);
}

void Gmax4002GpioStartSeq(void)
{
    int i = 0;

    /* RESET */
    /* reset gpio of reset pin */
    XGpio_DiscreteClear(&g_gpio, 1, GPIO_PIN_SENSOR_RESET_N);
    /* delay 2ms */
    DelayMs(2);
    /* set gpio of reset pin */
    XGpio_DiscreteSet(&g_gpio, 1, GPIO_PIN_SENSOR_RESET_N);

    /* START */
    for (i = 0; i < 2; i ++) {
        XGpio_DiscreteSet(&g_gpio, 1, GPIO_PIN_CTR_START);
        DelayMs(2);
        XGpio_DiscreteClear(&g_gpio, 1, GPIO_PIN_CTR_START);

        DelayMs(1100);
    }

    /* EXP ENABLE */
    XGpio_DiscreteSet(&g_gpio, 1, GPIO_PIN_CTR_EXP_EN);

    /* TEXP */
    XGpio_DiscreteSet(&g_gpio, 1, GPIO_PIN_SENSOR_TEXP);
    DelayMs(2);
    XGpio_DiscreteClear(&g_gpio, 1, GPIO_PIN_SENSOR_TEXP);
}
#endif
