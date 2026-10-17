/**
 ******************************************************************************
 * @file    gripper.h
 * @brief   夹爪电机控制 API（基于 PD42S1 闭环步进驱动器 Modbus-RTU 协议）
 *
 * PD42S1 Modbus-RTU 寄存器映射（摘自《正点原子步进电机驱动器modbus-rtu控制协议V1.2》）：
 *   读取类（功能码 0x04 读输入寄存器）：
 *     0x002A  电机实时位置    R   int32  (2 个寄存器，大端序，51200 脉冲 = 1 圈)
 *     0x002C  电机运行状态    R   uint8  (0停止 1完成 2运行 3过载 4堵转 5欠压)
 *     0x002D  电机堵转标志    R   uint8  (0未堵转 1堵转)
 *     0x002F  电机使能状态    R   uint8
 *     0x0030  电机到位标志    R   uint8  (0未到位 1到位)
 *   写单寄存器（功能码 0x06）：
 *     0x006A  设置堵转保护    W   0x0000关闭 / 0x0001开启
 *     0x00F9  解除堵转保护    W   固定 0x0000
 *     0x00FA  电机使能控制    W   0x0000失能 / 0x0001使能
 *     0x00FB  清除状态        W   固定 0x0000
 *     0x00FC  立即停止(刹车)  W   固定 0x0000
 *   写多寄存器（功能码 0x10）：
 *     0x00F2  绝对位置模式控制  W  4 个寄存器(8字节):
 *           dir(1B) + acc(1B) + speed(2B,大端,RPM) + pulses(4B,大端)
 *
 * 串口默认参数：115200 8N1，从机地址默认 1。
 * 所有多字节数据均为大端序（高字节在前）。
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

/* ======== 寄存器地址定义（PD42S1 真实地址） ======== */
#define REG_READ_POS             0x002AU   /* 电机实时位置（int32，大端） */
#define REG_READ_MOTOR_STA       0x002CU   /* 电机运行状态 */
#define REG_READ_CLOG_FLAG       0x002DU   /* 电机堵转标志 */
#define REG_READ_ENABLE_STA      0x002FU   /* 电机使能状态 */
#define REG_READ_ARRIVED_STA     0x0030U   /* 电机到位标志 */

#define REG_SET_CLOG_PRO         0x006AU   /* 设置堵转保护功能 */
#define REG_CLEAR_CLOG           0x00F9U   /* 解除堵转保护 */
#define REG_MOTOR_ENABLE         0x00FAU   /* 电机使能控制 */
#define REG_CLEAR_STATE          0x00FBU   /* 清除状态 */
#define REG_STOP_NOW             0x00FCU   /* 立即停止（刹车） */

#define REG_POS_MODE             0x00F2U   /* 绝对位置模式控制 */

#define MOTOR_ENABLED            1U
#define MOTOR_DISABLED           0U

/* 运行状态码 */
#define MOTOR_STA_STOP           0U   /* 停止 */
#define MOTOR_STA_COMPLETE       1U   /* 任务完成 */
#define MOTOR_STA_RUNNING        2U   /* 正在运行 */
#define MOTOR_STA_OVERLOAD       3U   /* 过载 */
#define MOTOR_STA_STALLED        4U   /* 堵转 */
#define MOTOR_STA_UNDERVOLTAGE   5U   /* 欠压 */

/* 夹爪开合脉冲定义（51200 脉冲 = 1 圈，按实际机械行程调整） */
#define GRIPPER_OPEN_PULSE       2000
#define GRIPPER_CLOSE_PULSE      0

/* 绝对位置模式运动参数 */
#define GRIPPER_POS_DIR          0U    /* 方向：0 正转 */
#define GRIPPER_POS_ACC          50U   /* 加速度（0~255，0 为直接运行） */
#define GRIPPER_POS_SPEED        300U  /* 速度（RPM） */

void Gripper_Init(void);

/**
 * @brief  绝对位置模式控制：设置目标脉冲并运动
 * @param  pulse  目标脉冲数（51200 = 1 圈）
 * @retval 1 成功，0 失败
 */
uint8_t Gripper_SetTargetPulse(int32_t pulse);

/** 读取电机实时位置（编码器脉冲） */
uint8_t Gripper_GetActualPos(int32_t *pos);

/** 读取电机运行状态（见 MOTOR_STA_xxx） */
uint8_t Gripper_GetMotorStatus(uint8_t *sta);

/** 读取堵转标志：0 未堵转，1 堵转 */
uint8_t Gripper_GetStallFlag(uint8_t *flag);

/** 读取到位标志：0 未到位，1 到位 */
uint8_t Gripper_GetArrivedFlag(uint8_t *flag);

/** 使能/失能电机：1=使能，0=失能 */
uint8_t Gripper_SetMotorEnable(uint8_t en);

/** 设置堵转保护功能：1=开启，0=关闭 */
uint8_t Gripper_SetStallProtect(uint8_t en);

/** 立即停止（刹车） */
uint8_t Gripper_StopNow(void);

/** 解除堵转保护 */
uint8_t Gripper_ClearStallProtect(void);

/** 清除状态（堵转/刹车/失能） */
uint8_t Gripper_ClearState(void);

#ifdef __cplusplus
}
#endif

#endif /* __GRIPPER_H */
