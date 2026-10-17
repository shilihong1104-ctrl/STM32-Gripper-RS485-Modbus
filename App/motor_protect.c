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
static uint8_t  g_stallActive = 0U;   /* 差值是否正在超限（替代 0 哨兵值，避免与 HAL_GetTick 上电 0 冲突） */
static uint32_t g_stallStartTick = 0U; /* 差值开始超限的时刻 */

void MotorProtect_Init(void)
{
    g_state = PROTECT_STATE_NORMAL;
    g_stallActive = 0U;
    g_stallStartTick = 0U;
}

ProtectState_t MotorProtect_Update(int32_t targetPulse, int32_t actualPos, uint8_t posValid)
{
    if (g_state == PROTECT_STATE_STALLED) {
        return g_state;
    }

    /* 通信失败时 actualPos 不可信，不进行堵转判定，保持当前计时状态不变 */
    if (posValid == 0U) {
        return g_state;
    }

    int32_t diff = targetPulse - actualPos;
    if (diff < 0) {
        diff = -diff;
    }

    if ((uint32_t)diff > PROTECT_DIFF_THRESHOLD) {
        /* 差值超限 */
        if (g_stallActive == 0U) {
            g_stallActive = 1U;
            g_stallStartTick = HAL_GetTick();   /* 记录起始时刻 */
        } else if ((HAL_GetTick() - g_stallStartTick) >= PROTECT_TIME_THRESHOLD_MS) {
            /* 持续超时，触发堵转保护：关闭电机 */
            Gripper_SetMotorEnable(MOTOR_DISABLED);
            g_state = PROTECT_STATE_STALLED;
        }
    } else {
        /* 差值回到阈值内，重置计时 */
        g_stallActive = 0U;
        g_stallStartTick = 0U;
    }

    return g_state;
}

void MotorProtect_Reset(void)
{
    g_state = PROTECT_STATE_NORMAL;
    g_stallActive = 0U;
    g_stallStartTick = 0U;
    /* 重新使能电机 */
    Gripper_SetMotorEnable(MOTOR_ENABLED);
}

void MotorProtect_TriggerStall(void)
{
    if (g_state != PROTECT_STATE_STALLED) {
        Gripper_SetMotorEnable(MOTOR_DISABLED);
        g_state = PROTECT_STATE_STALLED;
        g_stallActive = 0U;
        g_stallStartTick = 0U;
    }
}

ProtectState_t MotorProtect_GetState(void)
{
    return g_state;
}
