/**
 ******************************************************************************
 * @file    debug_uart.h
 * @brief   调试串口：printf 重定向到调试 UART（与 RS485 的 UART 分离）
 ******************************************************************************
 */
#ifndef __DEBUG_UART_H
#define __DEBUG_UART_H

#include "stm32u5xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_UART             USART1
#define DEBUG_UART_BAUDRATE    115200U

void DebugUart_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_UART_H */
