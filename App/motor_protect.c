/**
 ******************************************************************************
 * @file    motor_protect.c
 * @brief   电机堵转保护实现
 ******************************************************************************
 */
#include "motor_protect.h"
#include "gripper.h"
#include "stm32u5xx_hal.h"

static ProtectState_t g_state = PROTECT_STATE_NORMAL;
static uint32_t g_stallStartTick = 0U;   /* 差值开始超限的时刻 */

void MotorProtect_Init(void)
{
    g_state = PROTECT_STATE_NORMAL;
    g_stallStartTick = 0U;
}

ProtectState_t MotorProtect_Update(int32_t targetPulse, int32_t actualPos)
{
    if (g_state == PROTECT_STATE_STALLED) {
        return g_state;
    }

    int32_t diff = targetPulse - actualPos;
    if (diff < 0) {
        diff = -diff;
    }

    if ((uint32_t)diff > PROTECT_DIFF_THRESHOLD) {
        /* 差值超限 */
        if (g_stallStartTick == 0U) {
            g_stallStartTick = HAL_GetTick();   /* 记录起始时刻 */
        } else if ((HAL_GetTick() - g_stallStartTick) >= PROTECT_TIME_THRESHOLD_MS) {
            /* 持续超时，触发堵转保护：关闭电机 */
            Gripper_SetMotorEnable(MOTOR_DISABLED);
            g_state = PROTECT_STATE_STALLED;
        }
    } else {
        /* 差值回到阈值内，重置计时 */
        g_stallStartTick = 0U;
    }

    return g_state;
}

void MotorProtect_Reset(void)
{
    g_state = PROTECT_STATE_NORMAL;
    g_stallStartTick = 0U;
    /* 重新使能电机 */
    Gripper_SetMotorEnable(MOTOR_ENABLED);
}

ProtectState_t MotorProtect_GetState(void)
{
    return g_state;
}
