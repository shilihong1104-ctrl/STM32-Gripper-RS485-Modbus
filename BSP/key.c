/**
 ******************************************************************************
 * @file    key.c
 * @brief   按键驱动实现：10ms 周期消抖，下降沿触发一次按下事件
 ******************************************************************************
 */
#include "key.h"

#define KEY_DEBOUNCE_MS   30U   /* 消抖时间，单位 ms */

static uint8_t  keyStableLevel = KEY_PRESSED;  /* 稳定电平 */
static uint8_t  keyPrevLevel   = KEY_PRESSED;
static uint32_t keyLastChangeTick = 0U;
static uint8_t  keyFired = 0U;   /* 防止按住重复触发 */

void KEY_Init(void)
{
    keyStableLevel = KEY_PRESSED;
    keyPrevLevel   = KEY_PRESSED;
    keyLastChangeTick = HAL_GetTick();
    keyFired = 0U;
}

KeyEvent_t KEY_Process(void)
{
    KeyEvent_t event = KEY_EVENT_NONE;
    uint8_t curLevel = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
    uint32_t now = HAL_GetTick();

    if (curLevel != keyPrevLevel) {
        keyLastChangeTick = now;
        keyPrevLevel = curLevel;
    }

    /* 电平稳定超过消抖时间 */
    if ((now - keyLastChangeTick) >= KEY_DEBOUNCE_MS) {
        if (curLevel != keyStableLevel) {
            keyStableLevel = curLevel;

            /* 下降沿：从未按 -> 按下 */
            if ((keyStableLevel == KEY_PRESSED) && (!keyFired)) {
                event    = KEY_EVENT_PRESS;
                keyFired = 1U;
            }
            /* 上升沿：释放，重置触发标志 */
            if (keyStableLevel != KEY_PRESSED) {
                keyFired = 0U;
            }
        }
    }

    return event;
}
