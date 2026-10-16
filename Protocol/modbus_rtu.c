/**
 ******************************************************************************
 * @file    modbus_rtu.c
 * @brief   Modbus-RTU 主机实现
 ******************************************************************************
 */
#include "modbus_rtu.h"
#include "rs485.h"
#include <string.h>

#define MB_RESP_TIMEOUT_MS    50U    /* 应答超时 */
#define MB_MAX_FRAME_LEN      256U

/* ======== CRC16-Modbus 校验 ======== */
uint16_t Modbus_CRC16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i, j;

    for (i = 0U; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (j = 0U; j < 8U; j++) {
            if (crc & 0x0001U) {
                crc = (crc >> 1) ^ 0xA001U;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

/**
 * @brief  发送请求并接收应答（内部通用函数）
 * @param  req     请求帧（不含 CRC，CRC 由本函数追加）
 * @param  reqLen  请求帧长度（不含 CRC 字节）
 * @param  resp    应答缓冲区
 * @param  respMax 应答缓冲区最大长度
 * @retval >0 应答长度；0 超时
 */
static uint16_t Modbus_Transaction(const uint8_t *req, uint16_t reqLen, uint8_t *resp, uint16_t respMax)
{
    uint8_t  txBuf[MB_MAX_FRAME_LEN];
    uint16_t crc;

    if ((req == NULL) || (reqLen + 2U > MB_MAX_FRAME_LEN)) {
        return 0U;
    }

    /* 1. 打包请求帧：复制数据 + 追加 CRC（低字节在前） */
    memcpy(txBuf, req, reqLen);
    crc = Modbus_CRC16(txBuf, reqLen);
    txBuf[reqLen]     = (uint8_t)(crc & 0xFFU);       /* CRC 低字节 */
    txBuf[reqLen + 1U] = (uint8_t)((crc >> 8) & 0xFFU); /* CRC 高字节 */

    /* 2. 通过 RS485 发送 */
    RS485_Send(txBuf, reqLen + 2U);

    /* 3. 等待应答（轮询空闲中断组帧结果） */
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < MB_RESP_TIMEOUT_MS) {
        uint16_t rlen = RS485_GetFrame(resp, respMax);
        if (rlen >= 3U) {   /* 最短有效应答：地址 + 功能码 + CRC */
            return rlen;
        }
    }
    return 0U;
}

/**
 * @brief  校验应答帧的从机地址、功能码、CRC
 */
static MB_Status_t Modbus_VerifyResp(const uint8_t *resp, uint16_t respLen, uint8_t slave, uint8_t func)
{
    if (respLen < 5U) {
        return MB_ERR_LENGTH;
    }
    if (resp[0] != slave) {
        return MB_ERR_SLAVE;
    }
    if (resp[1] == (func | 0x80U)) {
        /* 异常响应：第 3 字节为异常码 */
        return MB_ERR_EXCEPTION;
    }
    if (resp[1] != func) {
        return MB_ERR_FUNC;
    }
    /* CRC 校验（低字节在前） */
    uint16_t crc_calc = Modbus_CRC16(resp, respLen - 2U);
    uint16_t crc_recv = (uint16_t)resp[respLen - 2U] | ((uint16_t)resp[respLen - 1U] << 8);
    if (crc_calc != crc_recv) {
        return MB_ERR_CRC;
    }
    return MB_OK;
}

/* ======== 读保持寄存器 0x03 ======== */
MB_Status_t Modbus_ReadHoldingRegs(uint8_t slave, uint16_t regAddr, uint16_t regCnt, uint16_t *outRegs)
{
    uint8_t req[6];
    uint8_t resp[MB_MAX_FRAME_LEN];
    MB_Status_t st;

    if ((regCnt == 0U) || (outRegs == NULL)) {
        return MB_ERR_LENGTH;
    }

    /* 请求：[Slave][03][RegHi][RegLo][NumHi][NumLo] */
    req[0] = slave;
    req[1] = MB_FUNC_READ_HOLDING;
    req[2] = (uint8_t)((regAddr >> 8) & 0xFFU);
    req[3] = (uint8_t)(regAddr & 0xFFU);
    req[4] = (uint8_t)((regCnt >> 8) & 0xFFU);
    req[5] = (uint8_t)(regCnt & 0xFFU);

    uint16_t rlen = Modbus_Transaction(req, 6U, resp, sizeof(resp));
    if (rlen == 0U) {
        return MB_ERR_TIMEOUT;
    }

    st = Modbus_VerifyResp(resp, rlen, slave, MB_FUNC_READ_HOLDING);
    if (st != MB_OK) {
        return st;
    }

    /* 应答：[Slave][03][ByteCount][Data...][CRC] */
    uint8_t byteCnt = resp[2];
    if (byteCnt != (uint8_t)(regCnt * 2U)) {
        return MB_ERR_LENGTH;
    }
    if (rlen != (uint16_t)(5U + byteCnt)) {
        return MB_ERR_LENGTH;
    }

    /* 大端转 uint16_t */
    for (uint16_t i = 0U; i < regCnt; i++) {
        outRegs[i] = ((uint16_t)resp[3U + i * 2U] << 8) | (uint16_t)resp[4U + i * 2U];
    }

    return MB_OK;
}

/* ======== 写单个寄存器 0x06 ======== */
MB_Status_t Modbus_WriteSingleReg(uint8_t slave, uint16_t regAddr, uint16_t value)
{
    uint8_t req[6];
    uint8_t resp[MB_MAX_FRAME_LEN];
    MB_Status_t st;

    /* 请求：[Slave][06][RegHi][RegLo][ValHi][ValLo] */
    req[0] = slave;
    req[1] = MB_FUNC_WRITE_SINGLE;
    req[2] = (uint8_t)((regAddr >> 8) & 0xFFU);
    req[3] = (uint8_t)(regAddr & 0xFFU);
    req[4] = (uint8_t)((value >> 8) & 0xFFU);
    req[5] = (uint8_t)(value & 0xFFU);

    uint16_t rlen = Modbus_Transaction(req, 6U, resp, sizeof(resp));
    if (rlen == 0U) {
        return MB_ERR_TIMEOUT;
    }

    st = Modbus_VerifyResp(resp, rlen, slave, MB_FUNC_WRITE_SINGLE);
    if (st != MB_OK) {
        return st;
    }

    /* 正常应答为请求回显，长度 8 */
    if (rlen != 8U) {
        return MB_ERR_LENGTH;
    }
    return MB_OK;
}

/* ======== 写多个寄存器 0x10 ======== */
MB_Status_t Modbus_WriteMultipleRegs(uint8_t slave, uint16_t regAddr, uint16_t regCnt, const uint16_t *values)
{
    uint8_t req[MB_MAX_FRAME_LEN];
    uint8_t resp[MB_MAX_FRAME_LEN];
    MB_Status_t st;
    uint16_t i;

    if ((regCnt == 0U) || (regCnt > 123U) || (values == NULL)) {
        return MB_ERR_LENGTH;
    }

    /* 请求：[Slave][10][RegHi][RegLo][NumHi][NumLo][ByteCnt][Data...] */
    req[0] = slave;
    req[1] = MB_FUNC_WRITE_MULTIPLE;
    req[2] = (uint8_t)((regAddr >> 8) & 0xFFU);
    req[3] = (uint8_t)(regAddr & 0xFFU);
    req[4] = (uint8_t)((regCnt >> 8) & 0xFFU);
    req[5] = (uint8_t)(regCnt & 0xFFU);
    req[6] = (uint8_t)(regCnt * 2U);
    for (i = 0U; i < regCnt; i++) {
        req[7U + i * 2U]     = (uint8_t)((values[i] >> 8) & 0xFFU);
        req[8U + i * 2U]     = (uint8_t)(values[i] & 0xFFU);
    }

    uint16_t rlen = Modbus_Transaction(req, 7U + regCnt * 2U, resp, sizeof(resp));
    if (rlen == 0U) {
        return MB_ERR_TIMEOUT;
    }

    st = Modbus_VerifyResp(resp, rlen, slave, MB_FUNC_WRITE_MULTIPLE);
    if (st != MB_OK) {
        return st;
    }
    /* 正常应答长度 8 */
    if (rlen != 8U) {
        return MB_ERR_LENGTH;
    }
    return MB_OK;
}
