# 夹爪电机控制系统

## 一、项目概述

基于 STM32U575 的夹爪电机控制程序，通过 RS485 + Modbus-RTU 协议与闭环步进电机驱动器通信，实现夹爪开合控制、位置监测与堵转保护。

**芯片：** STM32U575VGTx
**主频：** 160 MHz（HSE 8MHz → PLL）
**开发环境：** STM32CubeIDE（arm-none-eabi-gcc）
**通信协议：** Modbus-RTU（功能码 0x03 / 0x06 / 0x10）

---

## 二、文件结构

```
solution/
├── BSP/                    板级支持包（硬件抽象）
│   ├── rs485.c/h          RS485 驱动：UART + DE/RE 方向控制 + DMA IDLE 组帧
│   ├── key.c/h            按键驱动：10ms 消抖 + 下降沿单次触发
│   └── debug_uart.c/h     调试串口：printf 重定向到 USART1
├── Protocol/               协议层
│   └── modbus_rtu.c/h     Modbus-RTU 主机：CRC16 + 报文打包 + 应答解析
├── App/                    应用层
│   ├── gripper.c/h        夹爪电机控制（目标脉冲/实际位置/故障码/使能）
│   └── motor_protect.c/h  堵转保护（差值超时关电机）
├── Core/
│   └── main.c             主程序：按键切换开合 + 周期打印 + 堵转检测
├── CubeMX_配置步骤.md       STM32CubeMX 完整配置说明
├── GripperCtrl.ioc         STM32CubeMX 工程文件（可直接导入）
└── README.md               本文件
```

---

## 三、硬件引脚映射

| 引脚 | 功能 | 方向 | 说明 |
|------|------|------|------|
| PA2  | USART2_TX | 复用推挽 | RS485 发送（接 MAX485 DI） |
| PA3  | USART2_RX | 复用推挽 | RS485 接收（接 MAX485 RO） |
| PA4  | RS485_DE | 推挽输出 | 发送方向控制（1=发送, 0=接收） |
| PA5  | RS485_RE | 推挽输出 | 接收使能控制（0=使能接收, 1=禁用） |
| PA9  | USART1_TX | 复用推挽 | 调试串口打印 |
| PA10 | USART1_RX | 复用推挽 | 调试串口（预留） |
| PC13 | KEY | 输入上拉 | 按键（低电平有效） |

> MAX485 接线：PA2→DI，PA3→RO，PA4→DE，PA5→RE，AB 线接电机驱动器 RS485 总线。

---

## 四、环境准备

### 4.1 安装 STM32CubeIDE

1. 访问 https://www.st.com/zh/development-tools/stm32cubeide.html 下载最新版
2. 运行安装程序，按提示完成安装（建议安装路径不含中文和空格）
3. 首次启动时选择工作空间目录（Workspace），例如 `D:\STM32\workspace`

### 4.2 安装 STM32CubeMX

1. 访问 https://www.st.com/zh/development-tools/stm32cubemx.html 下载
2. 安装后需下载 STM32U5 的固件包（Help → Manage embedded software packages → 勾选 STM32U5 → Install）

---

## 五、CubeMX 配置与代码生成

### 步骤 1：打开工程文件

1. 双击 `GripperCtrl.ioc`，用 STM32CubeMX 打开
2. 确认芯片为 `STM32U575VGTx`

### 步骤 2：核对外设配置（无需修改，确认即可）

| 外设 | 配置 |
|------|------|
| RCC | HSE = Crystal/Ceramic Resonator |
| SYS | Debug = Serial Wire, Timebase = SysTick |
| USART1 | Asynchronous, 115200, 8N1, TX+RX (PA9/PA10) |
| USART2 | Asynchronous, 115200, 8N1, TX+RX (PA2/PA3) |
| GPDMA1 CH0 | USART2_RX, Circular, Peripheral→Memory |
| GPDMA1 CH1 | USART2_TX, Normal, Memory→Peripheral |
| NVIC | USART2 global, GPDMA1 CH0, GPDMA1 CH1 中断使能 |
| GPIO PA4 | Output Push Pull, No pull (RS485_DE) |
| GPIO PA5 | Output Push Pull, No pull (RS485_RE) |
| GPIO PC13 | Input, Pull-up (KEY) |
| Clock | HSE 8M → PLL (M=1, N=40, R=2) → SYSCLK 160MHz |

### 步骤 3：生成代码

1. 点击顶部 **Project Manager** 页签
2. 设置：
   - **Project Name**: `GripperCtrl`
   - **Toolchain / IDE**: `STM32CubeIDE`
3. 左侧点击 **Code Generator**，勾选：
   - ✅ Generate peripheral initialization as a pair of '.c/.h' files per peripheral
   - ✅ Keep User Code when re-generating
4. 点击右上角 **GENERATE CODE**
5. 弹出提示框点击 **Yes**，等待生成完成
6. 生成完成后点击 **Open Project**（自动用 STM32CubeIDE 打开）

---

## 六、添加业务代码到工程

代码生成后，工程结构中 `Core/Src/` 和 `Core/Inc/` 是 CubeMX 生成的基础文件。需要把本压缩包的业务代码合并进去。

### 步骤 1：复制文件

将压缩包中以下文件复制到生成的工程目录 `GripperCtrl/` 下：

| 源文件 | 目标位置 |
|--------|----------|
| `BSP/*.c` | `GripperCtrl/Core/Src/` |
| `BSP/*.h` | `GripperCtrl/Core/Inc/` |
| `Protocol/*.c` | `GripperCtrl/Core/Src/` |
| `Protocol/*.h` | `GripperCtrl/Core/Inc/` |
| `App/*.c` | `GripperCtrl/Core/Src/` |
| `App/*.h` | `GripperCtrl/Core/Inc/` |
| `Core/main.c` | 覆盖 `GripperCtrl/Core/Src/main.c` |

> 也可以保留 BSP/Protocol/App 目录结构，把 .c 文件复制到 Core/Src，.h 复制到 Core/Inc 即可。

### 步骤 2：配置头文件包含路径

1. 在 STM32CubeIDE 左侧 Project Explorer 中，右键 **GripperCtrl** → **Properties**
2. 左侧展开 **C/C++ Build** → **Settings**
3. 切换到 **Tool Settings** 标签
4. 找到 **MCU/MPU GCC Compiler** → **Include paths**
5. 点击右侧 **+** 按钮，依次添加以下 3 个路径：
   ```
   ../BSP
   ../Protocol
   ../App
   ```
   > 也可以点击 **Workspace...** 按钮，展开工程后选择对应文件夹
6. 点击 **Apply and Close**

### 步骤 3：刷新工程

在 Project Explorer 中右键工程 → **Refresh**（或按 F5），确认新增的 .c 文件已出现在 `Core/Src` 目录下。

---

## 七、编译

1. 点击工具栏的锤子图标 🔨（Build），或按快捷键 **Ctrl+B**
2. 等待底部控制台输出编译结果
3. 成功标志：
   ```
   Build Finished. 0 errors, 0 warnings.
   ```
4. 编译产物位于 `GripperCtrl/Debug/GripperCtrl.elf`（及 .bin / .hex）

**预期编译结果：** text ≈ 43 KB，data ≈ 104 B，bss ≈ 2.8 KB

### 常见编译问题排查

| 错误信息 | 原因 | 解决方法 |
|----------|------|----------|
| undefined reference to `RS485_Init` | .c 文件未加入编译 | 确认 .c 文件在 Core/Src 目录下，Refresh 工程 |
| `rs485.h: No such file` | 头文件路径未配置 | 检查 Include paths 是否添加了 BSP/Protocol/App |
| `handle_GPDMA1_Channel0` 未定义 | DMA 句柄名不匹配 | 确认 gpdma.c 中句柄名是否为 handle_GPDMA1_Channel0/1 |

---

## 八、烧录与运行

### 步骤 1：连接硬件

1. 用 ST-Link 连接开发板的 SWD 接口（SWDIO、SWCLK、GND、3.3V）
2. 用 USB 转串口模块连接 USART1（PA9=TX → 模块 RX，PA10=RX → 模块 TX，共地）
3. 连接 MAX485 与电机驱动器 RS485 总线

### 步骤 2：烧录程序

1. 点击工具栏绿色运行按钮 ▶（Run），或按 **F11** 进入 Debug
2. 首次运行会自动烧录固件到 Flash
3. 烧录完成后程序自动运行

### 步骤 3：查看串口输出

1. 打开串口助手，选择对应 COM 口
2. 波特率设为 **115200**，数据位 8，停止位 1，无校验
3. 上电后应看到输出：
   ```
   Gripper Controller Start...
   目标脉冲：0，实际位置：0，故障码：0
   目标脉冲：0，实际位置：0，故障码：0
   ...
   ```

### 步骤 4：测试功能

| 操作 | 预期结果 |
|------|----------|
| 按下按键（PC13） | 串口输出 `[KEY] Open -> target=2000`，目标脉冲变为 2000 |
| 再次按下按键 | 串口输出 `[KEY] Close -> target=0`，目标脉冲变为 0 |
| 电机堵转 2 秒 | 电机自动关闭（使能寄存器写 0） |

---

## 九、功能实现与考题对应

### 驱动底层能力

| 考题要求 | 实现文件 | 说明 |
|----------|----------|------|
| STM32 串口 + 485 DE/RE 方向控制 | `BSP/rs485.c` | DE/RE 引脚方向切换，发送前置 1，等待 TC 后清 0 |
| Modbus-RTU 报文打包 | `Protocol/modbus_rtu.c` | 0x03 读保持寄存器 / 0x06 写单寄存器 / 0x10 写多寄存器 |
| CRC16 校验 | `Protocol/modbus_rtu.c` | 多项式 0xA001，初始 0xFFFF，低字节在前 |
| 应答帧解析 | `Protocol/modbus_rtu.c` | 校验从机地址、功能码、异常码、CRC、字节数 |

### main 业务层

| 考题要求 | 实现位置 | 说明 |
|----------|----------|------|
| 按键切换开合（2000/0 脉冲循环） | `Core/main.c` App_Loop | 状态机：CLOSED↔OPENED，按键切换目标脉冲 |
| 串口持续打印 | `Core/main.c` App_Loop | 每 100ms 打印：目标脉冲、实际位置、故障码 |
| 堵转保护 | `App/motor_protect.c` | 差值 > 500 脉冲且持续 > 2000ms → 关闭电机 |

---

## 十、关键参数

| 参数 | 值 | 文件 |
|------|-----|------|
| RS485 波特率 | 115200 | rs485.h |
| 电机从机地址 | 1 | gripper.h |
| 张开脉冲 | 2000 | gripper.h |
| 闭合脉冲 | 0 | gripper.h |
| 堵转差值阈值 | 500 脉冲 | motor_protect.h |
| 堵转超时 | 2000 ms | motor_protect.h |
| 按键消抖 | 30 ms | key.c |
| 打印周期 | 100 ms | main.c |
| 按键扫描周期 | 10 ms | main.c |

---

## 十一、电机驱动器寄存器映射（PD42S1 闭环步进驱动器）

> 以下寄存器地址摘自《正点原子步进电机驱动器 modbus-rtu 控制协议 V1.2》，已对真实硬件手册校准。

### 读取类（功能码 0x04 读输入寄存器）

| 寄存器地址 | 名称 | 位宽 | 说明 |
|-----------|------|------|------|
| 0x002A | 电机实时位置 | int32 | 2 个寄存器，**大端序**，51200 脉冲 = 1 圈 |
| 0x002C | 电机运行状态 | uint8 | 0停止 1完成 2运行 3过载 4堵转 5欠压 |
| 0x002D | 电机堵转标志 | uint8 | 0 未堵转 / 1 堵转 |
| 0x002F | 电机使能状态 | uint8 | |
| 0x0030 | 电机到位标志 | uint8 | 0 未到位 / 1 到位 |

### 写单寄存器（功能码 0x06）

| 寄存器地址 | 名称 | 数据 | 说明 |
|-----------|------|------|------|
| 0x00F9 | 解除堵转保护 | 0x0000 | |
| 0x00FA | 电机使能控制 | 0/1 | 0 失能 / 1 使能 |
| 0x00FB | 清除状态 | 0x0000 | 清除堵转/刹车/失能状态 |
| 0x00FC | 立即停止(刹车) | 0x0000 | |

### 写多寄存器（功能码 0x10）

| 寄存器地址 | 名称 | 寄存器数 | 数据布局 |
|-----------|------|---------|---------|
| 0x00F2 | 绝对位置模式控制 | 4 | dir(1B) + acc(1B) + speed(2B大端,RPM) + pulses(4B大端) |

### 修改方法

打开 `App/gripper.h`，修改以下宏定义：

```c
#define GRIPPER_SLAVE_ADDR       1U        // 电机从机地址（默认 1）
#define REG_READ_POS             0x002AU   // 电机实时位置
#define REG_POS_MODE             0x00F2U   // 绝对位置模式控制
#define REG_MOTOR_ENABLE         0x00FAU   // 电机使能控制
#define REG_READ_CLOG_FLAG       0x002DU   // 电机堵转标志
```

---

## 十二、运行效果

串口助手连接 USART1（115200 8N1），上电后输出：

```
Gripper Controller Start...
目标脉冲：0，实际位置：0，状态：0，堵转：0，到位：1
目标脉冲：0，实际位置：0，状态：0，堵转：0，到位：1
...
```

按下按键（PC13）→ 目标脉冲变为 2000（张开），串口输出：
```
[KEY] Open -> target=2000
目标脉冲：2000，实际位置：XXX，状态：2，堵转：0，到位：0
```

再次按下 → 目标脉冲变为 0（闭合），循环切换。

堵转保护双保险：
1. **硬件堵转标志**（0x2D 寄存器）置位 → 立即关闭电机
2. **软件位置差保护**：目标与实际差值 > 500 持续 2 秒 → 关闭电机

堵转后再次按按键 → 自动解除堵转保护并使能电机，继续执行开合动作。
