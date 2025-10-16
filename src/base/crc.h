#ifndef CRC_H
#define CRC_H

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "xparameters.h"

#define CRC_POLY_16        0xA001
#define CRC_POLY_32        0xEDB88320ul
#define CRC_START_8        0x00
#define CRC_START_16    0x0000
#define CRC_START_32    0xFFFFFFFFul

int CheckCrc8(uint8_t *buffer, uint32_t len, uint8_t crc);
int CheckCrc16(uint8_t *buffer, uint32_t len, uint16_t crc);
int CheckCrc32(uint8_t *buffer, uint32_t len, uint32_t crc);

uint8_t CalcCrc8(const uint8_t *buf, uint32_t bytes);
uint16_t CalcCrc16(const uint8_t *buf, uint32_t bytes);
uint32_t CalcCrc32(const uint8_t *buf, uint32_t bytes);
uint16_t GencpCrc16(const uint8_t *data, uint32_t size);

#endif
