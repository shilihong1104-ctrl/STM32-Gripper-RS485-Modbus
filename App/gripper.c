/**
 ******************************************************************************
 * @file    gripper.c
 * @brief   夹爪电机控制实现
 ******************************************************************************
 */
#include "gripper.h"
#include "modbus_rtu.h"

void Gripper_Init(void)
{
    /* 默认使能电机 */
    Gripper_SetMotorEnable(MOTOR_ENABLED);
}

/**
 * @brief  写 32 位目标脉冲（2 个连续寄存器，低字在前）
 */
uint8_t Gripper_SetTargetPulse(int32_t pulse)
{
    uint16_t regs[2];
    regs[0] = (uint16_t)(pulse & 0xFFFFU);          /* 低字 */
    regs[1] = (uint16_t)((pulse >> 16) & 0xFFFFU);  /* 高字 */

    MB_Status_t st = Modbus_WriteMultipleRegs(GRIPPER_SLAVE_ADDR, REG_TARGET_PULSE, 2U, regs);
    return (st == MB_OK) ? 1U : 0U;
}

/**
 * @brief  读 32 位实际位置（2 个连续寄存器，低字在前）
 */
uint8_t Gripper_GetActualPos(int32_t *pos)
{
    uint16_t regs[2];
    MB_Status_t st = Modbus_ReadHoldingRegs(GRIPPER_SLAVE_ADDR, REG_ACTUAL_POS, 2U, regs);
    if (st != MB_OK) {
        return 0U;
    }
    *pos = (int32_t)((uint32_t)regs[0] | ((uint32_t)regs[1] << 16));
    return 1U;
}

/**
 * @brief  读故障码
 */
uint8_t Gripper_GetFaultCode(uint16_t *code)
{
    uint16_t reg;
    MB_Status_t st = Modbus_ReadHoldingRegs(GRIPPER_SLAVE_ADDR, REG_FAULT_CODE, 1U, &reg);
    if (st != MB_OK) {
        return 0U;
    }
    *code = reg;
    return 1U;
}

/**
 * @brief  电机使能控制
 */
uint8_t Gripper_SetMotorEnable(uint8_t en)
{
    MB_Status_t st = Modbus_WriteSingleReg(GRIPPER_SLAVE_ADDR, REG_MOTOR_ENABLE,
                                           (en) ? MOTOR_ENABLED : MOTOR_DISABLED);
    return (st == MB_OK) ? 1U : 0U;
}
