/*
 * config.h
 *
 *  Created on: 2025.2.27
 *      Author: Administrator
 */

#ifndef CONFIG_H
#define CONFIG_H

#define VERSION ("V1.25.42.02") /* year.week.num */

#define BOARD_JIUAN
//#define BOARD_3412
// #define BOARD_32152
// #define BOARD_248
// #define BOARD_201
// #define BOARD_ENDOSCOPE
// #define BOARD_PI
// #define BOARD_1402
// #define BOARD_2929

#ifdef BOARD_3412
/* VERSION */
#define BOARD    ("B3412") /* board 3412 */

/* FPGA */
// #define FPGA_FUDAN_MICRO
#define QIYI3412

/* MODULE */
// #define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0      0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_3412 */

#ifdef BOARD_JIUAN
/* VERSION */
#define BOARD    ("B_JIUAN") /* board jiuan */

/* FPGA */
#define FPGA_FUDAN_MICRO

/* MODULE */
#define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH
#define MODULE_SA_MF210A

/* FUNC */
#define FUNC_PASSTHROUGH

/* UART */
#ifdef MODULE_UART

#define UART_0          0
#define UART_1			1
#define UART_NUM_MAX    (UART_1 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_JIUAN */

#ifdef BOARD_248
/* VERSION */
#define BOARD    ("B248") /* board 248 */

/* FPGA */
// #define FPGA_FUDAN_MICRO

/* MODULE */
/* enable modules */
// #define MODULE_GENCP
// #define MODULE_UART
// #define MODULE_SPI_FLASH

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0     1
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_248 */


#ifdef BOARD_201
/* VERSION */
#define BOARD    ("B201") /* board 201 */

/* FPGA */
// #define FPGA_FUDAN_MICRO

/* MODULE */
/* enable modules */
// #define MODULE_GENCP
// #define MODULE_UART
// #define MODULE_SPI_FLASH

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0     0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_201 */


#ifdef BOARD_32152
/* VERSION */
#define BOARD    ("B32152")

/* FPGA */
#define FPGA_FUDAN_MICRO

/* MODULE */
// #define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0     0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_32152 */


#ifdef BOARD_ENDOSCOPE
/* VERSION */
#define BOARD    ("B_ENDOSCOPE")

/* FPGA */
// #define FPGA_FUDAN_MICRO

/* MODULE */
#define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH
#define ENDOSCOPE

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0      0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_B_ENDOSCOPE */


#ifdef BOARD_PI
/* VERSION */
#define BOARD    ("B_PI")

/* FPGA */
// #define FPGA_FUDAN_MICRO

/* MODULE */
// #define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH
// #define MODULE_MIPI
#define MODULE_I2C

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0     0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_B_PI */


#ifdef BOARD_1402
/* VERSION */
#define BOARD    ("B1402")

/* FPGA */
// #define FPGA_FUDAN_MICRO

/* MODULE */
#define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH
// #define MODULE_MIPI
#define LINESCAN
#define ISP_FFC
// #define ISP_DPC

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0      0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_B_PI */

#ifdef BOARD_2929
/* VERSION */
#define BOARD    ("B2929")

/* FPGA */
// #define FPGA_FUDAN_MICRO

/* MODULE */
#define MODULE_GENCP
#define MODULE_UART
#define MODULE_SPI_FLASH
// #define MODULE_MIPI
// #define LINESCAN
// #define ISP_FFC
// #define ISP_DPC
#define MODULE_USB3
#define CAM2929

/* UART */
#ifdef MODULE_UART

#define UART_0         0
#define UART_0      0
#define UART_NUM_MAX     (UART_0 + 1)

#endif /* MODULE_UART */

/* SPI */
#ifdef MODULE_SPI_FLASH

#include "xspi.h"
#define SPI_DEV_ID_CODE     XPAR_SPI_0_DEVICE_ID /* program */
#define SPI_DEV_ID_ARG      XPAR_SPI_0_DEVICE_ID /* arguments */
#define SPI_DEV_NUM_MAX     (SPI_DEV_ID_ARG + 1)

#endif /* MODULE_SPI_FLASH */

#endif /* BOARD_B_2929 */

#endif /* CONFIG_H */
