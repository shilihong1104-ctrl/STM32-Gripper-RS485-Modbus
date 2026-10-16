/**
 ******************************************************************************
 * @file    gripper.h
 * @brief   夹爪电机控制 API（基于 Modbus-RTU）
 *
 * 电机驱动寄存器映射（按实际驱动器手册调整，此处以常见闭环步进为例）：
 *   0x0000  目标脉冲（位置）  RW  int32  (2 个寄存器，低字在前)
 *   0x0002  实际位置（编码器）  R   int32  (2 个寄存器，低字在前)
 *   0x0004  故障码             R   uint16
 *   0x0006  电机使能           RW  uint16 (1=使能, 0=失能)
 ******************************************************************************
 */
#ifndef __GRIPPER_H
#define __GRIPPER_H

#include "stm32u5xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 电机从机地址 */
#define GRIPPER_SLAVE_ADDR       1U

/* 寄存器地址定义 */
#define REG_TARGET_PULSE        0x0000U   /* 目标脉冲（32位，占 0x0000/0x0001） */
#define REG_ACTUAL_POS          0x0002U   /* 实际位置（32位，占 0x0002/0x0003） */
#define REG_FAULT_CODE          0x0004U   /* 故障码 */
#define REG_MOTOR_ENABLE        0x0006U   /* 电机使能 */

#define MOTOR_ENABLED           1U
#define MOTOR_DISABLED          0U

/* 夹爪开合脉冲定义 */
#define GRIPPER_OPEN_PULSE      2000
#define GRIPPER_CLOSE_PULSE     0

void Gripper_Init(void);

/** 设置目标脉冲（位置） */
uint8_t Gripper_SetTargetPulse(int32_t pulse);

/** 读取实际位置（编码器脉冲） */
uint8_t Gripper_GetActualPos(int32_t *pos);

/** 读取故障码 */
uint8_t Gripper_GetFaultCode(uint16_t *code);

/** 使能/失能电机：1=使能，0=失能 */
uint8_t Gripper_SetMotorEnable(uint8_t en);

#ifdef __cplusplus
}
#endif

#endif /* __GRIPPER_H */
