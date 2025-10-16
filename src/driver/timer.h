#ifndef TIMER_H
#define TIMER_H

#include "xtmrctr.h"

#define XPAR_AXI_TIMER_DEVICE_ID    XPAR_AXI_TIMER_0_DEVICE_ID
#define XPAR_AXI_INTC_TIMER_IRQ_INTR    XPAR_INTC_0_TMRCTR_0_VEC_ID

extern XTmrCtr g_timerCtrl;

void vTickISR(void *callbackRef);

#endif /* TIMER_H */
