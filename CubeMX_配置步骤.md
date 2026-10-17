# STM32CubeMX 配置步骤文档

> 适用工程：夹爪电机控制系统
> 目标芯片：**STM32U575**（与工程中 `stm32u5xx_hal.h` 一致；若使用其它型号，引脚复用功能号需相应调整）
> 文档版本：v1.0
> 生成日期：2026-10-14

---

## 0. 引脚总览表

下表为本工程用到的全部引脚，配置前请先核对原理图是否一致。

| 引脚 | 功能 | 模式 | 上拉 | 复用功能 | 关联外设 |
|------|------|------|------|----------|----------|
| PA2 | USART2_TX | 复用推挽 | 无 | AF7 (USART2) | RS485 发送 |
| PA3 | USART2_RX | 复用推挽 | 无 | AF7 (USART2) | RS485 接收 |
| PA4 | RS485_DE | 推挽输出 | 无 | — | 485 方向控制 DE |
| PA5 | RS485_RE | 推挽输出 | 无 | — | 485 方向控制 RE |
| PA9 | USART1_TX | 复用推挽 | 无 | AF7 (USART1) | 调试串口发送 |
| PA10 | USART1_RX | 复用推挽 | 无 | AF7 (USART1) | 调试串口接收 |
| PC13 | KEY | 输入 | 上拉 | — | 按键（低电平有效） |

---

## 1. 新建工程

1. 打开 **STM32CubeMX** → 点击 **New Project**（或 File → New Project）。
2. 在 **Part Number** 搜索框输入 `STM32U575VGT6`（或你实际使用的封装型号）。
3. 在右侧列表选中对应芯片 → 点击右上角 **Start Project**。

---

## 2. RCC 时钟源配置

路径：左侧导航 **System Core → RCC**

| 配置项 | 值 | 说明 |
|--------|-----|------|
| High Speed Clock (HSE) | Crystal/Ceramic Resonator | 使用外部高速晶振 |
| Low Speed Clock (LSE) | Disable | 本工程不使用 RTC |
| HSE Divider for PLL1 (PLL1M) | 1 | 按实际晶振调整 |
| PLL1N | 20 | 配合 8MHz 晶振产生 160MHz |

> ⚠️ 若板载晶振不是 8MHz，请在后续 Clock Configuration 中重新计算 PLL 参数。

---

## 3. SYS 调试与时基

路径：**System Core → SYS**

| 配置项 | 值 |
|--------|-----|
| Debug | Serial Wire (SWD) |
| Timebase Source | SysTick |

---

## 4. USART1 调试串口（printf 输出）

路径：**Connectivity → USART1**

### 4.1 Mode 与参数

| 参数 | 值 | 对应代码 |
|------|-----|----------|
| Mode | Asynchronous | — |
| Baud Rate | 115200 | `DEBUG_UART_BAUDRATE` |
| Word Length | 8 Bits | `UART_WORDLENGTH_8B` |
| Parity | None | `UART_PARITY_NONE` |
| Stop Bits | 1 | `UART_STOPBITS_1` |
| Data Direction | **Receive and Transmit** | `UART_MODE_TX_RX` |
| Over Sampling | 16 Samples | `UART_OVERSAMPLING_16` |

### 4.2 引脚分配

| 引脚 | 功能 |
|------|------|
| PA9 | USART1_TX |
| PA10 | USART1_RX |

> ⚠️ 代码中 `huart1.Init.Mode = UART_MODE_TX_RX`，因此 **TX 和 RX 两个引脚必须同时配置**，不可只配 TX。

---

## 5. USART2 RS485 通信串口（核心外设）

路径：**Connectivity → USART2**

### 5.1 Mode 与参数

| 参数 | 值 | 对应代码 |
|------|-----|----------|
| Mode | Asynchronous | — |
| Baud Rate | 115200 | `RS485_UART_BAUDRATE` |
| Word Length | 8 Bits | `UART_WORDLENGTH_8B` |
| Parity | None | `UART_PARITY_NONE` |
| Stop Bits | 1 | `UART_STOPBITS_1` |
| Data Direction | **Receive and Transmit** | `UART_MODE_TX_RX` |
| Over Sampling | 16 Samples | `UART_OVERSAMPLING_16` |

### 5.2 引脚分配

| 引脚 | 功能 |
|------|------|
| PA2 | USART2_TX |
| PA3 | USART2_RX |

### 5.3 GPDMA1 配置（STM32U5 系列使用 GPDMA，非普通 DMA）

> ⚠️ STM32U575 的 DMA 为 **GPDMA1**（General Purpose DMA），配置方式与 F1/F4 等系列不同。
> 在 USART2 的 **DMA Settings** 中会提示 "USART2 requests should be configured in GPDMA1"，点击 **Go to GPDMA1** 按钮跳转。

#### 步骤 1：分配通道

在 GPDMA1 通道列表中，为 USART2_RX 和 USART2_TX 各分配一个通道：

| 通道 | Mode | Request |
|------|------|---------|
| Channel 0 | **Standard Request Mode** | USART2_RX |
| Channel 1 | **Standard Request Mode** | USART2_TX |

> 点击通道右侧的 Mode 下拉框，从 `Disable` 改为 `Standard Request Mode`，然后在 Request 中选择对应请求。

#### 步骤 2：CH0（USART2_RX）参数

切到 **CH0** 标签页，按下表配置：

| 参数分组 | 参数 | 值 |
|----------|------|-----|
| Circular configuration | Circular Mode | **Enable** ⚠️ |
| Request Configuration | Request | USART2_RX |
| Channel configuration | Priority | Medium |
| Channel configuration | Direction | Peripheral To Memory |
| Source Data Setting | Source Address Increment After Transfer | Disabled |
| Source Data Setting | Data Width | Byte |
| Source Data Setting | Burst Length | 1 |
| Destination Data Setting | Destination Address Increment After Transfer | **Enabled** |
| Destination Data Setting | Data Width | Byte |
| Destination Data Setting | Burst Length | 1 |

#### 步骤 3：CH1（USART2_TX）参数

切到 **CH1** 标签页，按下表配置：

| 参数分组 | 参数 | 值 |
|----------|------|-----|
| Circular configuration | Circular Mode | Disable |
| Request Configuration | Request | USART2_TX |
| Channel configuration | Priority | Low（或 Medium） |
| Channel configuration | Direction | Memory To Peripheral |
| Source Data Setting | Source Address Increment After Transfer | **Enabled** |
| Source Data Setting | Data Width | Byte |
| Source Data Setting | Burst Length | 1 |
| Destination Data Setting | Destination Address Increment After Transfer | Disabled |
| Destination Data Setting | Data Width | Byte |
| Destination Data Setting | Burst Length | 1 |

> 关键：**CH0（USART2_RX）的 Circular Mode 必须设为 Enable**，代码中使用 `HAL_UARTEx_ReceiveToIdle_DMA` 实现循环接收 + 空闲中断组帧。

### 5.4 NVIC 中断配置

路径：**System Core → NVIC**

| 中断 | 使能 | 优先级（抢占/子） |
|------|------|-------------------|
| GPDMA1 Channel 0 global interrupt | ✅ 勾选 | 0 / 0 |
| GPDMA1 Channel 1 global interrupt | ✅ 勾选 | 0 / 0 |
| USART2 global interrupt | ✅ 勾选 | 0 / 0 |

> - USART2 全局中断用于检测 **IDLE 空闲线中断**，从而识别一帧 Modbus 报文结束。
> - GPDMA1 CH0 中断：USART2_RX DMA 完成通知。
> - GPDMA1 CH1 中断：USART2_TX DMA 完成通知。

---

## 6. GPIO 配置（DE/RE 方向控制 + 按键）

路径：**System Core → GPIO**

在右侧芯片引脚图上点击对应引脚，或在 GPIO 列表中添加：

| 引脚 | GPIO mode | Pull-up/Pull-down | Maximum output speed | User Label |
|------|-----------|-------------------|----------------------|------------|
| PA4 | Output Push Pull | No pull | Low | RS485_DE |
| PA5 | Output Push Pull | No pull | Low | RS485_RE |
| PC13 | Input | Pull-up | — | KEY |

> - PA4/PA5：RS485 收发方向控制。`DE=1 & RE=1` 发送，`DE=0 & RE=0` 接收。Maximum output speed 选 Low 即可（方向切换频率不高），选 High 也可以。
> - PC13：按键输入，低电平有效（按下时为 GND），内部上拉。

---

## 7. NVIC 全局配置

路径：**System Core → NVIC**

确认以下中断已使能：

| 中断 | 使能 |
|------|------|
| USART1 global interrupt | ❌ 不使能（printf 阻塞发送，无需中断） |
| USART2 global interrupt | ✅ 使能（IDLE 帧结束检测） |
| GPDMA1 Channel 0 global interrupt | ✅ 使能（USART2_RX DMA） |
| GPDMA1 Channel 1 global interrupt | ✅ 使能（USART2_TX DMA） |

---

## 8. Clock Configuration 时钟树

进入顶部 **Clock Configuration** 页签，按以下参数配置：

1. **Input frequency (HSE)**: `8` MHz（按实际晶振修改）
2. **PLL1 Source Mux**: `HSE`
3. **PLL1M**: `/1`
4. **PLL1N**: `x20`
5. **PLL1P**: `/1`
6. **SYSCLK Mux**: `PLL1CLK`
7. **HCLK**: `160` MHz（U575 最高主频）
8. **APB1 Prescaler**: `/1`（PCLK1 = 160 MHz）
9. **APB2 Prescaler**: `/1`（PCLK2 = 160 MHz）
10. **APB3 Prescaler**: `/1`（PCLK3 = 160 MHz）
11. **USART1 Clock Mux**: `PCLK2`
12. **USART2 Clock Mux**: `PCLK1`

> 最终目标：**SYSCLK = 160 MHz，HCLK = 160 MHz，PCLK1 = PCLK2 = PCLK3 = 160 MHz**。
> 确保 APB 时钟 ≥ 16 MHz，以保证 115200 波特率的精度。
> 若 APB 设为 /1 后出现红色警告，保持默认值即可（100 MHz 也满足波特率要求）。

---

## 9. Project Manager 工程设置

### 9.1 Project 页签

| 配置项 | 值 |
|--------|-----|
| Project Name | 自定义（如 GripperCtrl） |
| Project Location | 自定义路径 |
| Toolchain / IDE | `MDK-ARM`（Keil）或 `STM32CubeIDE` |
| MCU Reference | 自动填充 |

### 9.2 Code Generator 页签

勾选以下项：

- ✅ **Generate peripheral initialization as a pair of '.c/.h' files per peripheral**（外设初始化分文件生成）
- ✅ **Keep User Code when re-generating**（重新生成时保留用户代码）
- ✅ **Set all free pins as analog**（可选，降低功耗）

---

## 10. 生成代码

1. 点击右上角 **GENERATE CODE** 按钮。
2. 等待生成完成 → 点击 **Open Project** 打开工程。
3. 将 `solution/` 目录下的 `BSP/`、`Protocol/`、`App/` 文件夹及 `Core/main.c` 加入工程编译。

---

## 11. 生成代码后核对清单

生成代码后，请对照下表核对关键配置值（仅核对关键字段，非逐行比对）。

### 11.1 串口参数核对

| 核对项 | 期望值 | 核对位置 |
|--------|--------|----------|
| USART1 波特率 | 115200 | `DEBUG_UART_BAUDRATE` |
| USART2 波特率 | 115200 | `RS485_UART_BAUDRATE` |
| USART1/2 数据位 | 8 Bits | `UART_WORDLENGTH_8B` |
| USART1/2 校验位 | None | `UART_PARITY_NONE` |
| USART1/2 停止位 | 1 | `UART_STOPBITS_1` |

### 11.2 GPIO 核对

| 核对项 | 期望值 | 核对位置 |
|--------|--------|----------|
| DE 引脚端口 | GPIOA | `RS485_DE_GPIO_Port` |
| DE 引脚号 | PIN_4 | `RS485_DE_Pin` |
| RE 引脚端口 | GPIOA | `RS485_RE_GPIO_Port` |
| RE 引脚号 | PIN_5 | `RS485_RE_Pin` |
| DE/RE 模式 | 推挽输出 | `GPIO_MODE_OUTPUT_PP` |
| 按键引脚 | GPIOC PIN_13 | `KEY_Pin` |
| 按键模式 | 输入上拉 | `GPIO_MODE_INPUT + GPIO_PULLUP` |

### 11.3 GPDMA1 与中断核对

| 核对项 | 期望值 |
|--------|--------|
| GPDMA1 CH0 Request | USART2_RX |
| GPDMA1 CH0 Circular Mode | Enable |
| GPDMA1 CH0 Direction | Peripheral To Memory |
| GPDMA1 CH0 Dest Addr Increment | Enabled |
| GPDMA1 CH1 Request | USART2_TX |
| GPDMA1 CH1 Circular Mode | Disable |
| GPDMA1 CH1 Direction | Memory To Peripheral |
| GPDMA1 CH1 Source Addr Increment | Enabled |
| USART2 全局中断 | 使能 |
| GPDMA1 CH0 中断 | 使能 |
| GPDMA1 CH1 中断 | 使能 |

### 11.4 中断服务函数

CubeMX 会在 `stm32u5xx_it.c` 中自动生成以下中断服务函数，确认其存在并调用 HAL 库处理函数：

- `USART2_IRQHandler()` → 调用 `HAL_UART_IRQHandler(&huart2)`
- `GPDMA1_Channel0_IRQHandler()` → 调用 `HAL_DMA_IRQHandler(&hdma_usart2_rx)`
- `GPDMA1_Channel1_IRQHandler()` → 调用 `HAL_DMA_IRQHandler(&hdma_usart2_tx)`

> 代码中 `rs485.c` 的 `HAL_UARTEx_RxEventCallback` 回调会在 USART2 接收到数据或检测到 IDLE 时被 HAL 自动调用，无需手动注册。

---

## 12. 常见问题排查

### Q1：USART2 收不到数据
- 检查 DE/RE 引脚是否正确配置为输出，且空闲时为低电平（接收态）。
- 检查 GPDMA1 CH0（USART2_RX）是否配置为 Circular 模式。
- 检查 USART2 全局中断和 GPDMA1 CH0 中断是否使能。
- 确认代码中调用了 `HAL_UARTEx_ReceiveToIdle_DMA` 启动接收。

### Q2：printf 无输出
- 确认 USART1 的 PA9/PA10 引脚已配置且复用功能正确。
- 确认工程中 `debug_uart.c` 的 `_write` / `fputc` 重定向函数已加入编译。

### Q3：Modbus 通信超时
- 检查 485 接线 A/B 是否接反。
- 检查从机地址、波特率是否与驱动器一致。
- 用示波器抓取 DE 引脚，确认发送时 DE 为高电平，发送完成后及时拉低。

### Q4：按键无响应
- 确认 PC13 配置为输入上拉。
- 确认按键硬件为低电平有效（按下接 GND）。

---

## 13. PD42S1 驱动器接线说明

### 13.1 STM32 ↔ MAX485 ↔ PD42S1 接线表

| STM32 引脚 | 功能 | MAX485 引脚 | PD42S1 驱动器端子 |
|-----------|------|------------|------------------|
| PA2 (USART2_TX) | 串口发送 | DI (数据输入) | — |
| PA3 (USART2_RX) | 串口接收 | RO (数据输出) | — |
| PA4 | 发送使能 DE | DE | — |
| PA5 | 接收使能 RE | RE (与 DE 可短接) | — |
| 5V | 电源 | VCC | — |
| GND | 地 | GND | GND |
| — | — | A (非反相输出) | A (RS485+) |
| — | — | B (反相输出) | B (RS485-) |

### 13.2 PD42S1 驱动器侧端子

PD42S1 的 RS485 通信端子为 `A` 和 `B`（差分信号），务必与 MAX485 的 A/B 对应连接，接反将导致通信失败。

- 默认从机地址：**1**
- 默认串口波特率：**115200**，8N1
- 驱动器需先使能 Modbus-RTU 协议模式（寄存器 0x69 设为 1），默认即为 Modbus 模式。

### 13.3 关键提醒

1. **A/B 不能接反**：接反后从机不应答，Modbus 报超时。
2. **共地**：STM32 与 PD485 与驱动器必须共 GND，否则差分信号参考电平错误。
3. **DE/RE 控制**：本工程 PA4(DE) 与 PA5(RE) 分别控制，发送前同时拉高，等 USART TC 置位后再拉低，确保最后一字节完整发出。
4. **终端电阻**：总线较长（>10m）时在 A/B 之间并接 120Ω 终端电阻。
