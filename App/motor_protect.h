/**
 ******************************************************************************
 * @file    motor_protect.h
 * @brief   电机堵转保护：持续对比目标脉冲与实际位置，差值长时间超阈值则关电机
 *
 * 堵转判定双保险：
 *   1. 位置差超时（本模块实现）：目标与实际位置差值持续超限超过时间阈值
 *   2. 驱动器硬件堵转标志（上层 main.c 读取 0x2D 寄存器）
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
 * @param  posValid     实际位置是否有效（通信成功时为 1，失败时为 0）
 *                      posValid=0 时不进行堵转判定，避免通信失败导致误判
 * @retval 当前保护状态
 */
ProtectState_t MotorProtect_Update(int32_t targetPulse, int32_t actualPos, uint8_t posValid);

/** 重置堵转保护（故障排除后调用） */
void MotorProtect_Reset(void);

/** 外部触发堵转保护（如硬件堵转标志置位时调用），立即关电机并锁定状态 */
void MotorProtect_TriggerStall(void);

/** 获取当前保护状态 */
ProtectState_t MotorProtect_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_PROTECT_H */
