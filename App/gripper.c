/**
 ******************************************************************************
 * @file    gripper.c
 * @brief   夹爪电机控制实现（PD42S1 Modbus-RTU）
 ******************************************************************************
 */
#include "gripper.h"
#include "modbus_rtu.h"

void Gripper_Init(void)
{
    /* 上电先清除状态（堵转/刹车/失能） */
    Gripper_ClearState();
    /* 开启驱动器硬件堵转保护（0x6A=1），确保堵转时驱动器能置位 0x2D 标志 */
    Gripper_SetStallProtect(1U);
    /* 使能电机 */
    Gripper_SetMotorEnable(MOTOR_ENABLED);
}

/**
 * @brief  绝对位置模式控制（功能码 0x10，写 4 个寄存器 = 8 字节）
 *         数据：dir(1B) + acc(1B) + speed(2B大端) + pulses(4B大端)
 */
uint8_t Gripper_SetTargetPulse(int32_t pulse)
{
    uint16_t regs[4];

    /* 寄存器 0：dir(高字节) + acc(低字节) */
    regs[0] = ((uint16_t)GRIPPER_POS_DIR << 8) | (uint16_t)GRIPPER_POS_ACC;
    /* 寄存器 1：speed（RPM，大端） */
    regs[1] = (uint16_t)GRIPPER_POS_SPEED;
    /* 寄存器 2,3：pulses（大端，高字在前） */
    regs[2] = (uint16_t)(((uint32_t)pulse >> 16) & 0xFFFFU);
    regs[3] = (uint16_t)((uint32_t)pulse & 0xFFFFU);

    MB_Status_t st = Modbus_WriteMultipleRegs(GRIPPER_SLAVE_ADDR, REG_POS_MODE, 4U, regs);
    return (st == MB_OK) ? 1U : 0U;
}

/**
 * @brief  读电机实时位置（功能码 0x04，2 个寄存器，大端序 int32）
 */
uint8_t Gripper_GetActualPos(int32_t *pos)
{
    uint16_t regs[2];
    MB_Status_t st = Modbus_ReadInputRegs(GRIPPER_SLAVE_ADDR, REG_READ_POS, 2U, regs);
    if (st != MB_OK) {
        return 0U;
    }
    /* 大端序：regs[0] 是高 16 位，regs[1] 是低 16 位 */
    *pos = (int32_t)(((uint32_t)regs[0] << 16) | (uint32_t)regs[1]);
    return 1U;
}

/**
 * @brief  读电机运行状态（功能码 0x04，1 个寄存器，取低字节）
 */
uint8_t Gripper_GetMotorStatus(uint8_t *sta)
{
    uint16_t reg;
    MB_Status_t st = Modbus_ReadInputRegs(GRIPPER_SLAVE_ADDR, REG_READ_MOTOR_STA, 1U, &reg);
    if (st != MB_OK) {
        return 0U;
    }
    *sta = (uint8_t)(reg & 0xFFU);
    return 1U;
}

/**
 * @brief  读堵转标志（功能码 0x04，1 个寄存器）
 */
uint8_t Gripper_GetStallFlag(uint8_t *flag)
{
    uint16_t reg;
    MB_Status_t st = Modbus_ReadInputRegs(GRIPPER_SLAVE_ADDR, REG_READ_CLOG_FLAG, 1U, &reg);
    if (st != MB_OK) {
        return 0U;
    }
    *flag = (uint8_t)(reg & 0xFFU);
    return 1U;
}

/**
 * @brief  读到到位标志（功能码 0x04，1 个寄存器）
 */
uint8_t Gripper_GetArrivedFlag(uint8_t *flag)
{
    uint16_t reg;
    MB_Status_t st = Modbus_ReadInputRegs(GRIPPER_SLAVE_ADDR, REG_READ_ARRIVED_STA, 1U, &reg);
    if (st != MB_OK) {
        return 0U;
    }
    *flag = (uint8_t)(reg & 0xFFU);
    return 1U;
}

/**
 * @brief  电机使能控制（功能码 0x06）
 */
uint8_t Gripper_SetMotorEnable(uint8_t en)
{
    uint16_t value = (en) ? (uint16_t)MOTOR_ENABLED : (uint16_t)MOTOR_DISABLED;
    MB_Status_t st = Modbus_WriteSingleReg(GRIPPER_SLAVE_ADDR, REG_MOTOR_ENABLE, value);
    return (st == MB_OK) ? 1U : 0U;
}

/**
 * @brief  设置堵转保护功能（功能码 0x06，0x6A）
 * @param  en  1=开启堵转保护，0=关闭
 */
uint8_t Gripper_SetStallProtect(uint8_t en)
{
    uint16_t value = (en) ? 0x0001U : 0x0000U;
    MB_Status_t st = Modbus_WriteSingleReg(GRIPPER_SLAVE_ADDR, REG_SET_CLOG_PRO, value);
    return (st == MB_OK) ? 1U : 0U;
}

/**
 * @brief  立即停止/刹车（功能码 0x06）
 */
uint8_t Gripper_StopNow(void)
{
    MB_Status_t st = Modbus_WriteSingleReg(GRIPPER_SLAVE_ADDR, REG_STOP_NOW, 0x0000U);
    return (st == MB_OK) ? 1U : 0U;
}

/**
 * @brief  解除堵转保护（功能码 0x06）
 */
uint8_t Gripper_ClearStallProtect(void)
{
    MB_Status_t st = Modbus_WriteSingleReg(GRIPPER_SLAVE_ADDR, REG_CLEAR_CLOG, 0x0000U);
    return (st == MB_OK) ? 1U : 0U;
}

/**
 * @brief  清除状态（功能码 0x06）
 */
uint8_t Gripper_ClearState(void)
{
    MB_Status_t st = Modbus_WriteSingleReg(GRIPPER_SLAVE_ADDR, REG_CLEAR_STATE, 0x0000U);
    return (st == MB_OK) ? 1U : 0U;
}
