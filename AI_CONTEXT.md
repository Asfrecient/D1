# AI_CONTEXT.md

# 项目概览

这是一个基于 `STM32F103C8T6 + FreeRTOS` 的环境监测实习项目，当前主线已经从“采集 + 显示 + Shell 调试”推进到“运行时配置 + 掉电保存配置”。

当前版本：

```text
v3.0.1 Storage Config Save/Load
```

当前完成：

* `BME280` 温湿度气压采集
* `SSD1306 OLED` 双页面显示
* `FreeRTOS` 多任务架构
* `Queue / Mutex / Semaphore / Software Timer / Event Flags`
* `UART Shell`
* `Logger System` + Ring Buffer 历史记录
* `Config System` 运行时参数管理
* `app_storage` 内部 Flash 配置持久化
* `save` 命令保存配置
* 上电自动加载已保存配置，未保存时回退默认值

---

# 硬件与工具

硬件：

```text
MCU: STM32F103C8T6
Sensor: BME280 (I2C)
Display: SSD1306 OLED (I2C)
Key: PA0 / GPIO_EXTI0
UART: USART1
```

预留但未开发：

```text
ESP8266 NodeMCU
W25Q64 Flash
蜂鸣器
```

工具：

```text
STM32CubeMX
STM32CubeIDE
CLion
ST-Link
CMake
```

---

# 工程分层

采用：

```text
Driver + App + RTOS
```

主要模块：

```text
App:
  app_sensor
  app_display
  app_shell
  app_logger
  app_config
  app_storage

Drivers:
  bme280
  oled

RTOS:
  freertos.c
```

职责：

* `Driver`：外设驱动与寄存器读写
* `App`：业务逻辑、显示、日志、配置、存储、Shell
* `RTOS`：任务和同步对象创建调度

约束：

* 调用关系保持 `Task -> App -> Driver`
* 业务逻辑不直接堆进 CubeMX 生成文件

---

# 当前功能状态

OLED：

* `Page0`：温度 / 湿度 / 气压
* `Page1`：Task Stack / Free Heap
* 页面切换时才清屏，减少闪屏

按键：

* `PA0 EXTI` 切换页面
* `50ms` 软件消抖

Logger：

* 每次采样后记录一条历史数据
* 支持 `log` 查看历史
* 支持 `clear` 清空历史

Config：

* 当前配置项：`sampleIntervalMs`
* `set interval <ms>` 修改采样周期
* `Config_SetSampleInterval()` 会重启 `SensorTimer` 立即生效

Storage：

* `Config_Init()` 启动时先尝试加载已保存配置
* 无有效配置时使用默认值 `1000ms`
* `save` 命令把当前配置写入 MCU 内部 Flash
* 当前持久化地址：`0x0800FC00`
* 使用 `magic` 校验保存数据是否合法

支持命令：

```text
help
version
all
temp
hum
press
stack
heap
log
clear
show config
config
set interval <ms>
save
```

---

# FreeRTOS 状态

任务：

```text
SensorTask
DisplayTask
ShellTask
MonitorTask
```

对象：

```text
Queue: shellQueueHandle
Queue: sensorQueueHandle
Mutex: I2CMutexHandle
Semaphore: SensorSemHandle
SoftwareTimer: SensorTimerHandle
EventFlags: DisplayEventHandle
```

设计原则：

* 采样链路优先级高于显示和调试
* 中断只负责接收/通知，不做复杂逻辑
* 运行时配置统一通过 `Config` 模块管理
* 持久化由 `Storage` 模块负责，不让 Shell 直接操作 Flash 细节

---

# 关键链路

采样链路：

```text
SensorTimer
  -> SensorSem
  -> SensorTask
  -> APP_SensorRead()
  -> Logger_Record()
  -> sensorQueue
  -> DisplayTask
  -> OLED
```

配置链路：

```text
ShellTask
  -> APP_ShellProcess()
  -> Config_SetSampleInterval()
  -> SensorTimer stop/start
```

持久化链路：

```text
ShellTask
  -> save
  -> Storage_SaveConfig()
  -> HAL_FLASHEx_Erase()
  -> HAL_FLASH_Program()
```

启动加载链路：

```text
Config_Init()
  -> Storage_LoadConfig()
  -> 有效配置: 直接使用
  -> 无效配置: sampleIntervalMs = 1000
```

---

# 下一阶段

```text
v3.1 Storage Robustness
```

目标：

* `save` 命令返回值失败处理
* `load/reset` 命令
* 更多配置项持久化
* 掉电恢复与边界情况验证
