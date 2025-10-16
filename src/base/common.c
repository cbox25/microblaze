/*
 * common.c
 *
 *  Created on: 2024.4.22
 *      Author: wwy
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "cmd.h"
#include "common.h"

uint16_t ConvertEndian16(uint16_t data)
{
    return ((data & 0xFF00) >> 8) | ((data & 0x00FF) << 8);
}

uint32_t ConvertEndian32(uint32_t data)
{
    return ((data & 0x000000FF) << 24 |
        (data & 0x0000FF00) << 8 |
        (data & 0x00FF0000) >> 8 |
        (data & 0xFF000000) >> 24);
}

uint64_t ConvertEndian64(uint64_t data)
{
    return ((data & 0x00000000000000FF) << 56) |
        ((data & 0x000000000000FF00) << 40) |
        ((data & 0x0000000000FF0000) << 24) |
        ((data & 0x00000000FF000000) << 8) |
        ((data & 0x000000FF00000000) >> 8) |
        ((data & 0x0000FF0000000000) >> 24) |
        ((data & 0x00FF000000000000) >> 40) |
        ((data & 0xFF00000000000000) >> 56);
}

__attribute__((optimize("O0"))) /* must not be deleted*/
static inline void DelayOneUs(void)
{
    for (volatile int i = 0; i < 8; i++) {
        ;
    }
}

__attribute__((optimize("O0"))) /* must not be deleted*/
static inline void DelayOneMs(void)
{
    for (volatile int i = 0; i < 8340; i++) {
        ;
    }
}

__attribute__((optimize("O0"))) /* must not be deleted*/
void DelayUs(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        DelayOneUs();
    }
}

__attribute__((optimize("O0"))) /* must not be deleted*/
void DelayMs(uint32_t ms)
{
    if (ms == 0) {
        return;
    }

    for (uint32_t i = 0; i < ms; i++) {
        DelayOneMs();
    }
}

void WriteReg(Reg *reg)
{
    *(volatile uint32_t *)reg->addr = reg->value;
    DelayMs(reg->delay);
}

void ReadReg(Reg *reg)
{
    reg->value = *(volatile uint32_t *)reg->addr;
}

void WriteRegByAddr(uint32_t addr, uint32_t value)
{
    *(volatile uint32_t *)addr = value;
}

uint32_t ReadRegByAddr(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

uint8_t U32ToDecStr(uint32_t val, uint8_t *out)
{
    char tmp[11]; /* 2^32 will not exceed 10 byte string */
    uint8_t i = 0;

    do {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    } while (val);

    uint8_t len = i;
    while (i--) {
        *out++ = tmp[i];
    }
    *out = '\0';

    return len;
}

#ifdef ENDOSCOPE
uint32_t WriteRegOah428(uint32_t addr, uint32_t value)
{
	uint32_t data = 0;

	data |= (addr & 0xFFFF) << 8;
	data |= value & 0xFF;

	*(volatile uint32_t *)(AXI_BASE_ADDR + 0xD70) = data;

	LOG_DEBUG("endoscope write reg, a: 0x%x, v: 0x%x\r\n", addr, value);
}

uint32_t ReadRegOah428(uint32_t addr)
{
	uint32_t value = 0;
	/* write addr */
	*(volatile uint32_t *)(AXI_BASE_ADDR + 0xD74) = addr & 0xFFFF;
	DelayMs(2);

	/* read data */
	value = *(volatile uint32_t *)AXI_BASE_ADDR + 0xD74);

	LOG_DEBUG("endoscope read reg, a: 0x%x, v: 0x%x\r\n", addr, value);

	return value;
}
#endif

void PrintBuf(uint8_t *buf, int len, const char *name)
{
#ifdef __DEBUG
    if (buf == NULL || len < 0 || name == NULL)
        return;

    xil_printf("--- %s ---\r\n", name);
    for (int i = 0; i < len; i ++) {
        xil_printf("0x%02X, ", buf[i]);
        if ((((i + 1) & 0x1F)) == 0) {
            xil_printf("\r\n");
        } else if (((i + 1) & 0x7) == 0) {
        	xil_printf("    ");
        }
    }
    xil_printf("\r\n");
#else
    ;
#endif
}
