/**
 ******************************************************************************
 * @file    key.h
 * @brief   按键驱动：消抖 + 按下事件回调（单击切换）
 ******************************************************************************
 */
#ifndef __KEY_H
#define __KEY_H

#include "stm32u5xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_GPIO_Port    GPIOC
#define KEY_Pin          GPIO_PIN_13
#define KEY_PRESSED      GPIO_PIN_RESET   /* 低电平有效 */

/** 按键事件类型 */
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESS       /* 一次有效按下（消抖后） */
} KeyEvent_t;

void KEY_Init(void);

/**
 * @brief  周期调用（建议 10ms），返回是否产生按下事件
 * @retval KEY_EVENT_PRESS 表示检测到一次有效按下；KEY_EVENT_NONE 无事件
 */
KeyEvent_t KEY_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */
