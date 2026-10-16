/**
 ******************************************************************************
 * @file    rs485.c
 * @brief   RS485 驱动实现：UART + DE/RE 方向控制 + DMA 收发 + 空闲中断组帧
 ******************************************************************************
 */
#include "rs485.h"
#include <string.h>

/* ======== 句柄（由 CubeMX 生成，定义在 usart.c） ======== */
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef  handle_GPDMA1_Channel0;   /* USART2_RX */
extern DMA_HandleTypeDef  handle_GPDMA1_Channel1;   /* USART2_TX */

/* DMA 循环接收缓冲区 */
static uint8_t  rxBuf[RS485_RX_BUF_SIZE];
static volatile uint8_t  frameReady = 0;
static volatile uint16_t frameLen = 0;

/* ======== 内部函数 ======== */
static void RS485_SetDirTx(void)
{
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);   /* DE=1 发送 */
    HAL_GPIO_WritePin(RS485_RE_GPIO_Port, RS485_RE_Pin, GPIO_PIN_SET);   /* RE=1 禁用接收 */
}

static void RS485_SetDirRx(void)
{
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET); /* DE=0 接收 */
    HAL_GPIO_WritePin(RS485_RE_GPIO_Port, RS485_RE_Pin, GPIO_PIN_RESET); /* RE=0 使能接收 */
}

void RS485_Init(void)
{
    /* DE/RE GPIO 已在 MX_GPIO_Init 中配置为推挽输出，此处仅确保默认处于接收态 */
    RS485_SetDirRx();

    /* 启动 DMA 循环接收（IDLE 中断组帧） */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxBuf, RS485_RX_BUF_SIZE);
}

void RS485_Send(const uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) {
        return;
    }

    /* 1. 切换为发送方向 */
    RS485_SetDirTx();

    /* 2. 发送数据 */
    HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 100U);

    /* 3. 等待 TC（发送移位寄存器空），防止最后一字节被截断 */
    uint32_t start = HAL_GetTick();
    while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET) {
        if ((HAL_GetTick() - start) > 10U) {
            break;
        }
    }

    /* 4. 切回接收方向 */
    RS485_SetDirRx();
}

uint16_t RS485_GetFrame(uint8_t *buf, uint16_t max)
{
    if (!frameReady) {
        return 0U;
    }

    uint16_t len = frameLen;
    if (len > max) {
        len = max;
    }

    /* 当前实现：rxBuf 存放下一帧，简单帧模型 */
    memcpy(buf, rxBuf, len);

    /* 清标志，准备接收下一帧 */
    frameReady = 0U;
    frameLen   = 0U;

    return len;
}

/**
 * @brief  IDLE 中断回调：在 HAL_UARTEx_RxEventCallback 中调用
 *         Size 为本次接收到的字节数（由 HAL 通过 NDTR 计算得出）
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == RS485_UART) {
        frameLen   = Size;
        frameReady = 1U;

        /* 重新启动 DMA 接收下一帧 */
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxBuf, RS485_RX_BUF_SIZE);
    }
}

void RS485_IDLE_Callback(void)
{
    /* 若使用手动 IDLE 中断方式，在此处理；本实现用 HAL_UARTEx_RxEventCallback */
}

void RS485_DMARxCpltCallback(void)
{
    /* DMA 全满回调，已在 RxEventCallback 中统一处理 */
}
