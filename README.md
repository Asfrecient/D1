# STM32 环境监测系统

基于 `STM32F103C8T6` 的嵌入式环境监测项目，使用 `BME280` 采集温度、湿度和气压，通过 `SSD1306 OLED` 显示，并结合 `FreeRTOS` 实现多任务调度、消息通信和资源同步。

当前版本：`v2.0`

## 项目目标

- 完成 BME280 传感器驱动与补偿算法
- 在 OLED 上实时显示环境数据
- 使用 FreeRTOS 组织采样、显示、调试和监控任务
- 通过 Queue、Mutex、Semaphore、Timer 和 Event Flags 构建稳定的任务协作链路
- 提供 UART Shell 方便运行时调试

## 硬件平台

- MCU: `STM32F103C8T6`
- 传感器: `BME280`
- 显示屏: `SSD1306 OLED`
- 通信接口: `I2C1`
- 调试串口: `USART1`
- 按键输入: `PA0 / EXTI0`

## 软件架构

项目采用 `Driver + App + RTOS` 分层：

- `Driver` 负责寄存器访问、I2C 读写和外设底层驱动
- `App` 负责业务逻辑、数据处理、显示和 Shell 命令处理
- `RTOS` 负责任务创建、调度与同步对象管理

主调用链路：

```text
SensorTimer
  -> SensorSem
  -> SensorTask
  -> sensorQueue
  -> DisplayTask
  -> OLED
```

按键链路：

```text
Key(EXTI0)
  -> HAL_GPIO_EXTI_Callback
  -> osEventFlagsSet(DisplayEventHandle, DISPLAY_EVENT_KEY)
  -> DisplayTask
  -> 页面切换
```

Shell 链路：

```text
USART1 RX Interrupt
  -> shellRxBuffer
  -> shellQueue
  -> ShellTask
  -> APP_ShellProcess(cmd)
```

## 当前功能

### 传感器采集

- BME280 温度补偿
- BME280 湿度补偿
- BME280 气压补偿
- 原始数据到显示值的格式转换

### OLED 显示

- 页面 0: 温度 / 湿度 / 气压
- 页面 1: Stack Monitor / Free Heap
- 仅在页面切换时清屏，减少闪屏

### FreeRTOS 任务

- `SensorTask`：负责采样与数据更新
- `DisplayTask`：负责 OLED 刷新与页面切换
- `ShellTask`：负责串口命令解析
- `MonitorTask`：负责后台状态监控

### RTOS 对象

- `Queue`
- `Mutex`
- `Semaphore`
- `Software Timer`
- `Event Flags`

### UART Shell 命令

- `help`
- `version`
- `all`
- `temp`
- `hum`
- `press`
- `stack`
- `heap`

示例输出：

```text
EnvMonitor v2.0
T:25.36C
H:42.58%
P:1008.01hPa
```

## 目录结构

```text
App/
├── Inc/
│   ├── app_display.h
│   ├── app_sensor.h
│   ├── app_shared.h
│   └── app_shell.h
└── Src/
    ├── app_display.c
    ├── app_sensor.c
    └── app_shell.c

Core/
├── Inc/
│   ├── bme280.h
│   ├── oled.h
│   └── ...
└── Src/
    ├── bme280.c
    ├── freertos.c
    ├── oled.c
    └── ...
```

## 开发环境

- STM32CubeMX
- STM32CubeIDE
- CLion
- ST-Link
- CMake
- ARM GCC Toolchain

## 运行说明

1. 用 STM32CubeMX / STM32CubeIDE 生成或同步工程配置
2. 用支持 ARM GCC 的工具链编译工程
3. 下载到 `STM32F103C8T6`
4. 通过 `USART1` 连接串口终端，输入 `help` 查看命令

## 学习记录

- [AI_CONTEXT.md](/Users/as/STM32/week1/AI_CONTEXT.md)
- [LEARNING_LOG.md](/Users/as/STM32/week1/LEARNING_LOG.md)
- [BME280学习笔记.md](/Users/as/STM32/week1/BME280学习笔记.md)

## 已知版本

- `v1.0`：BME280 + OLED + FreeRTOS + Mutex 双任务架构
- `v2.0`：Queue、Timer、Semaphore、Event Flags、UART Shell、页面切换优化

