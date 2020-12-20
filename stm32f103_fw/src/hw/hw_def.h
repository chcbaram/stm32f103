/*
 * hw_def.h
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"



#define _HW_DEF_RTOS_MEM_SIZE(x)              ((x)/4)


#define _HW_DEF_RTOS_THREAD_PRI_MAIN          osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_LED           osPriorityNormal


#define _HW_DEF_RTOS_THREAD_MEM_MAIN          _HW_DEF_RTOS_MEM_SIZE( 2*1024)
#define _HW_DEF_RTOS_THREAD_MEM_LED           _HW_DEF_RTOS_MEM_SIZE(    256)



#define _USE_HW_RTOS
#define _USE_HW_USB
#define _USE_HW_RTC
#define _USE_HW_RESET
#define _USE_HW_CDC
#define _USE_HW_FLASH



#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         2

#define _USE_HW_CLI
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_CMD_LIST_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    32



#endif /* SRC_HW_HW_DEF_H_ */
