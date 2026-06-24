# STM32 环境监测系统

基于 `STM32F103C8T6` 的嵌入式环境监测项目。系统使用 `BME280` 采集温度、湿度和气压，通过 `SSD1306 OLED` 实时显示，并借助 `FreeRTOS` 完成多任务调度、消息通信、配置管理与配置持久化。

当前版本：`v3.0.1`

## 项目亮点

- `BME280` 温度 / 湿度 / 气压补偿算法完整实现
- `OLED` 双页面显示，兼顾环境数据和系统运行状态
- `FreeRTOS` 多任务架构清晰，采样、显示、Shell、监控职责分离
- `Queue + Mutex + Semaphore + Software Timer + Event Flags` 组合使用
- `UART Shell` 支持运行时查询、调试、参数修改
- `Logger System` 支持历史记录查询与清空
- `Config System` 支持采样周期运行时修改
- `app_storage` 支持配置写入 MCU 内部 Flash，实现掉电保存

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
- `App` 负责采样、显示、日志、配置、存储和 Shell 逻辑
- `RTOS` 负责任务创建、调度与同步对象管理

核心采样链路：

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

配置持久化链路：

```text
set interval <ms>
  -> Config_SetSampleInterval()
  -> SensorTimer stop/start

save
  -> Storage_SaveConfig()
  -> HAL_FLASHEx_Erase()
  -> HAL_FLASH_Program()
```

启动加载链路：

```text
Config_Init()
  -> Storage_LoadConfig()
  -> valid: 使用保存值
  -> invalid: 使用默认值 1000ms
```

## 功能清单

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

### Logger 与配置

- `log`：查看历史采样记录
- `clear`：清空日志缓冲区
- `show config` / `config`：查看当前配置
- `set interval <ms>`：修改采样周期
- `save`：保存当前配置到内部 Flash

### 当前 Shell 命令

- `help`
- `version`
- `all`
- `temp`
- `hum`
- `press`
- `stack`
- `heap`
- `log`
- `clear`
- `show config`
- `config`
- `set interval <ms>`
- `save`

## 持久化设计

当前仅保存一个配置项：

```text
sampleIntervalMs
```

存储方式：

- 使用 MCU 内部 Flash 最后一页附近地址 `0x0800FC00`
- 写入结构包含 `magic + AppConfig_t`
- 上电先检查 `magic`，通过后才恢复配置
- 当前默认采样周期为 `1000ms`

这样可以实现：

- 运行中用 `set interval` 修改周期
- 确认无误后用 `save` 写入 Flash
- 系统重启后自动恢复上次保存的周期

## 目录结构

```text
App/
├── Inc/
│   ├── app_config.h
│   ├── app_display.h
│   ├── app_logger.h
│   ├── app_sensor.h
│   ├── app_shared.h
│   ├── app_shell.h
│   └── app_storage.h
└── Src/
    ├── app_config.c
    ├── app_display.c
    ├── app_logger.c
    ├── app_sensor.c
    ├── app_shell.c
    └── app_storage.c

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

1. 用 STM32CubeMX / STM32CubeIDE 生成或同步工程配置。
2. 使用支持 ARM GCC 的工具链编译工程。
3. 下载到 `STM32F103C8T6` 开发板。
4. 通过 `USART1` 连接串口终端，输入 `help` 查看命令。
5. 需要掉电保存采样周期时，先执行 `set interval <ms>`，再执行 `save`。

## 文档入口

- [AI_CONTEXT.md](/Users/as/STM32/week1/AI_CONTEXT.md)
- [LEARNING_LOG.md](/Users/as/STM32/week1/LEARNING_LOG.md)

## 版本演进

- `v1.0`：BME280 + OLED + FreeRTOS 基础双任务架构
- `v2.0`：Queue、Timer、Semaphore、Event Flags、UART Shell、页面切换优化
- `v2.1`：Logger System + Ring Buffer + `log/clear`
- `v2.2`：Config System + 采样周期运行时修改
- `v3.0.1`：`app_storage` + 配置保存/加载 + `save` 命令
