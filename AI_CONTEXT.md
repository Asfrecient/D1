# AI_CONTEXT.md

# 项目概览

这是一个基于 STM32 + FreeRTOS 的嵌入式实习项目，目标是逐步实现环境监测与物联网系统。

当前版本：

```text
v2.1 Logger System
```

当前已完成：

* BME280 温湿度气压采集
* OLED 实时显示
* FreeRTOS 多任务架构
* Queue 任务间通信
* Mutex 保护 I2C 总线
* Software Timer + Semaphore 事件驱动采样
* EXTI 按键输入
* Event Flags 事件通知
* OLED 双页面显示
* 按键软件消抖
* DisplayTask 响应优化
* OLED 刷新优化
* UART Shell
* 命令解析
* Shell Command System
* 系统状态查询
* 传感器数据查询
* 任务栈查询
* Logger System
* 环形日志缓冲区
* log / clear 历史记录命令

---

# 硬件与工具

MCU：

```text
STM32F103C8T6
```

模块：

```text
OLED SSD1306（I2C）
BME280（I2C）
按键（PA0 / GPIO_EXTI0，已接入）
USART1（UART Shell）
ESP8266 NodeMCU（未开发）
W25Q64 Flash（未开发）
蜂鸣器（未开发）
```

开发工具：

```text
STM32CubeMX
STM32CubeIDE
CLion
ST-Link
```

---

# 工程分层

采用：

```text
Driver + App + RTOS
```

主要文件：

```text
App/
  app_sensor.c
  app_sensor.h
  app_display.c
  app_display.h
  app_shell.c
  app_shell.h
  app_logger.c
  app_logger.h

Drivers/
  bme280.c
  bme280.h
  oled.c
  oled.h

FreeRTOS/
  freertos.c
```

职责：

* Driver：寄存器读写、I2C、OLED/BME280 驱动，不依赖 FreeRTOS
* App：业务逻辑、数据处理、显示与命令处理
* RTOS：任务、队列、同步对象创建与调度

约束：

* 业务逻辑不要直接写进 CubeMX 管理文件
* 调用关系保持 `Task -> App -> Driver`

---

# 当前功能状态

BME280：

* 温度 / 湿度 / 气压补偿算法已完成
* 输出单位与显示格式已处理

OLED：

* Page0：温度、湿度、气压
* Page1：Stack Monitor / Free Heap
* 仅在页面切换时 `OLED_Clear()`，避免闪屏

按键：

* PA0 外部中断切换页面
* 50ms 软件消抖

UART Shell：

* USART1 RX Interrupt 接收命令
* ShellTask 解析命令并调用 `APP_ShellProcess()`
* 命令通过 Queue 从中断侧转给任务侧处理

支持命令：

```text
help
version
all
temp
hum
press
stack
log
clear
```

输出示例：

```text
EnvMonitor v2.1 Logger System
T:xx.xxC
H:xx.xx%
P:xxxx.xxhPa
```

---

# FreeRTOS状态

任务：

```text
SensorTask   Normal
DisplayTask  BelowNormal
ShellTask    Low
MonitorTask  Low
```

RTOS对象：

```text
Queue:          sensorQueueHandle
Queue:          shellQueueHandle
Mutex:          I2CMutexHandle
Semaphore:      SensorSemHandle
SoftwareTimer:  SensorTimerHandle
EventFlags:     DisplayEventHandle
```

已使用：

```text
Task
Queue
Mutex
Semaphore
Software Timer
Event Flags
Ring Buffer
```

设计原则：

* 采样优先于显示
* 显示优先于调试输出
* Shell 负责运行时调试，不影响采样主链路
* `MonitorTask` 属于后台诊断任务，不应影响采样和显示

---

# 关键数据与架构

Queue数据结构：

```c
typedef struct
{
    int32_t temperature;
    uint32_t humidity;
    uint32_t pressure;
} BME280_Data_t;
```

关键状态：

* 使用 Queue 传递传感器数据
* 已删除全局 `sensor` 共享变量
* 新增 `BME280_Data_t g_latestData;` 用于 Shell 查询和状态展示
* `g_latestData` 不替代 Queue
* 新增 `LoggerRecord_t` 保存 `tick + data`
* `g_logBuffer[]` / `g_logWriteIndex` / `g_logCount` 管理历史日志
* 每次采样后调用 `Logger_Record()`

主链路：

```text
SensorTimer
  -> SensorSem
  -> SensorTask
  -> Logger_Record()
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
  -> g_displayPage ^= 1
  -> OLED Page Switch
```

Shell链路：

```text
USART1 RX Interrupt
  -> shellRxBuffer
  -> shellQueue
  -> ShellTask
  -> APP_ShellProcess(cmd)
```

Logger链路：

```text
SensorTask
  -> APP_SensorRead(&txData)
  -> g_latestData
  -> Logger_Record(&txData)
  -> Ring Buffer
```

说明：

* 采样采用 `Timer + Semaphore` 事件驱动
* Shell 命令通过 Message Queue 从中断侧传递到任务侧
* Logger 采用 Ring Buffer 保存固定容量历史记录
* `log` 命令按时间顺序输出最旧到最新的历史采样
* `clear` 命令清空日志缓冲区
