/**
 ******************************************************************************
 * @file    motor_protect.h
 * @brief   电机堵转保护：持续对比目标脉冲与实际位置，差值长时间超阈值则关电机
 ******************************************************************************
 */
#ifndef __MOTOR_PROTECT_H
#define __MOTOR_PROTECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 保护参数（可按实际调整） */
#define PROTECT_DIFF_THRESHOLD      500U    /* 位置差阈值（脉冲） */
#define PROTECT_TIME_THRESHOLD_MS   2000U   /* 差值持续超限的时间阈值（ms） */

/** 堵转保护状态 */
typedef enum {
    PROTECT_STATE_NORMAL = 0,
    PROTECT_STATE_STALLED        /* 已触发堵转，电机被关闭 */
} ProtectState_t;

void MotorProtect_Init(void);

/**
 * @brief  周期调用，更新堵转保护状态
 * @param  targetPulse  当前目标脉冲
 * @param  actualPos    实际编码器位置
 * @retval 当前保护状态
 */
ProtectState_t MotorProtect_Update(int32_t targetPulse, int32_t actualPos);

/** 重置堵转保护（故障排除后调用） */
void MotorProtect_Reset(void);

/** 获取当前保护状态 */
ProtectState_t MotorProtect_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_PROTECT_H */
