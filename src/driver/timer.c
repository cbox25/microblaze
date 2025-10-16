#include "Timer.h"
#include "FreeRTOS.h"
#include "task.h"

XTmrCtr g_timerCtrl;

void vTickISR(void *callbackRef)
{
    XTmrCtr_Reset(&g_timerCtrl, 0);
    if (xTaskIncrementTick() != pdFALSE) {
        vTaskSwitchContext();
    }
}
