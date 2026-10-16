/**
 ******************************************************************************
 * @file    rs485.h
 * @brief   RS485 硬件抽象层：UART + DE/RE 方向引脚控制
 *          基于 STM32 HAL 库，使用 DMA 收发以降低 CPU 占用。
 ******************************************************************************
 */
#ifndef __RS485_H
#define __RS485_H

#include "stm32u5xx_hal.h"   /* 根据实际芯片替换：stm32f1xx_hal.h / stm32f4xx_hal.h 等 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======== 硬件引脚配置（按实际原理图修改） ======== */
#define RS485_UART              USART2
#define RS485_UART_BAUDRATE     115200U
#define RS485_UART_IRQn         USART2_IRQn

/* DE/RE 方向控制引脚，通常接在一起：DE=1 & RE=1 发送；DE=0 & RE=0 接收 */
#define RS485_DE_GPIO_Port      GPIOA
#define RS485_DE_Pin            GPIO_PIN_4
#define RS485_RE_GPIO_Port      GPIOA
#define RS485_RE_Pin            GPIO_PIN_5

/* 接收缓冲区大小 */
#define RS485_RX_BUF_SIZE       256U

/**
 * @brief  RS485 初始化：UART + DE/RE GPIO + DMA + 空闲中断
 */
void RS485_Init(void);

/**
 * @brief  RS485 发送指定长度数据（自动切换 DE/RE 方向，发送完成自动切回接收）
 * @param  data  数据指针
 * @param  len   数据长度
 */
void RS485_Send(const uint8_t *data, uint16_t len);

/**
 * @brief  获取当前接收到的一帧数据（由空闲中断触发帧结束）
 * @param  buf   输出缓冲区
 * @param  max   输出缓冲区最大长度
 * @retval 实际帧长度，0 表示无新帧
 */
uint16_t RS485_GetFrame(uint8_t *buf, uint16_t max);

/**
 * @brief  UART 空闲中断回调中调用，标记一帧接收完成
 */
void RS485_IDLE_Callback(void);

/**
 * @brief  DMA 接收完成回调（半满/全满）中调用
 */
void RS485_DMARxCpltCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* __RS485_H */
