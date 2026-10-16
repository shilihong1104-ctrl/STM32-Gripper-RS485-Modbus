/**
 ******************************************************************************
 * @file    debug_uart.c
 * @brief   调试串口实现 + printf 重定向（重写 _write / fputc）
 ******************************************************************************
 */
#include "debug_uart.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;

void DebugUart_Init(void)
{
    /* huart1 已由 MX_USART1_UART_Init 初始化，此处可扩展 */
}

/* ======== printf 重定向 ======== */
#if defined(__GNUC__)
int _write(int fd, char *ptr, int len)
{
    (void)fd;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, (uint16_t)len, 100U);
    return len;
}
#else
int fputc(int ch, FILE *f)
{
    (void)f;
    uint8_t c = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &c, 1U, 100U);
    return ch;
}
#endif
