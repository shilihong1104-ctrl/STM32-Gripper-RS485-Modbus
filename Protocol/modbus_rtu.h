/**
 ******************************************************************************
 * @file    modbus_rtu.h
 * @brief   Modbus-RTU 主机协议：报文打包、CRC16 校验、应答帧解析
 *
 * 支持功能码：
 *   0x03 读保持寄存器
 *   0x04 读输入寄存器（PD42S1 状态/位置读取使用此功能码）
 *   0x06 写单个寄存器
 *   0x10 写多个寄存器
 ******************************************************************************
 */
#ifndef __MODBUS_RTU_H
#define __MODBUS_RTU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Modbus 功能码 */
#define MB_FUNC_READ_HOLDING      0x03U
#define MB_FUNC_READ_INPUT        0x04U
#define MB_FUNC_WRITE_SINGLE      0x06U
#define MB_FUNC_WRITE_MULTIPLE    0x10U

/* 错误码（应答最高位置 1） */
#define MB_EXCEPTION_ILLEGAL_FUNC     0x01U
#define MB_EXCEPTION_ILLEGAL_ADDR     0x02U
#define MB_EXCEPTION_ILLEGAL_VALUE    0x03U

/* 主机执行结果 */
typedef enum {
    MB_OK = 0,
    MB_ERR_TIMEOUT,
    MB_ERR_CRC,
    MB_ERR_SLAVE,        /* 从机地址不匹配 */
    MB_ERR_FUNC,         /* 功能码不匹配（含异常响应） */
    MB_ERR_LENGTH,       /* 帧长度异常 */
    MB_ERR_EXCEPTION,    /* 从机返回异常码 */
} MB_Status_t;

/**
 * @brief  计算 Modbus CRC16（多项式 0xA001，初始 0xFFFF）
 */
uint16_t Modbus_CRC16(const uint8_t *data, uint16_t len);

/**
 * @brief  读保持寄存器（功能码 0x03）
 * @param  slave    从机地址
 * @param  regAddr  起始寄存器地址
 * @param  regCnt   读取寄存器数量
 * @param  outRegs  输出寄存器值数组（大端已转换为 uint16_t）
 * @retval MB_OK 或错误码
 */
MB_Status_t Modbus_ReadHoldingRegs(uint8_t slave, uint16_t regAddr, uint16_t regCnt, uint16_t *outRegs);

/**
 * @brief  读输入寄存器（功能码 0x04）
 * @note   PD42S1 驱动器的位置/状态读取均使用此功能码
 */
MB_Status_t Modbus_ReadInputRegs(uint8_t slave, uint16_t regAddr, uint16_t regCnt, uint16_t *outRegs);

/**
 * @brief  写单个寄存器（功能码 0x06）
 */
MB_Status_t Modbus_WriteSingleReg(uint8_t slave, uint16_t regAddr, uint16_t value);

/**
 * @brief  写多个寄存器（功能码 0x10）
 */
MB_Status_t Modbus_WriteMultipleRegs(uint8_t slave, uint16_t regAddr, uint16_t regCnt, const uint16_t *values);

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_RTU_H */
