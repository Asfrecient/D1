# STM32F103 + FreeRTOS + BME280 环境监测系统

## 项目简介

基于 STM32F103C8T6 开发的嵌入式环境监测系统。

项目使用 BME280 传感器采集温度、湿度和气压数据，通过 SSD1306 OLED 实时显示，并基于 FreeRTOS 实现多任务调度与资源管理。

本项目重点在于：

- BME280 驱动开发
- Bosch 官方补偿算法实现
- FreeRTOS 多任务设计
- I2C 总线资源共享
- Mutex 互斥锁同步机制

## 硬件平台

### MCU

- STM32F103C8T6

### 传感器

- BME280
  - 温度
  - 湿度
  - 气压

### 显示器

- SSD1306 OLED

### 通信接口

- I2C1

BME280 与 OLED 共用同一条 I2C 总线。

## 软件架构

```text
+-------------------+
|    DisplayTask    |
+---------+---------+
          |
          v
      OLED SSD1306

+-------------------+
|    SensorTask     |
+---------+---------+
          |
          v
        BME280

     FreeRTOS
          |
          v
      I2C Mutex
          |
          v
       I2C1 Bus
```

## 项目结构

```text
App
├── Inc
│   └── app_display.h
│
└── Src
    └── app_display.c

Core
├── Inc
│   ├── bme280.h
│   ├── oled.h
│   └── ...
│
└── Src
    ├── bme280.c
    ├── freertos.c
    ├── oled.c
    └── ...
```

## 已实现功能

### BME280 驱动

实现：

- `BME280_Init()`
- `BME280_ReadTemperature()`
- `BME280_ReadHumidity()`
- `BME280_ReadPressure()`
- `BME280_ReadData()`

支持：

- 温度补偿
- 湿度补偿
- 气压补偿
- 校准参数自动解析

### OLED 显示

实时显示：

```text
T:25.36C
H:42.58%
P:1008.01hPa
```

支持：

- 负温度显示
- 小数显示
- 自动格式化输出

### FreeRTOS

创建两个任务：

#### SensorTask

功能：

- 周期读取 BME280 数据
- 更新传感器数据结构

优先级：`Normal`

#### DisplayTask

功能：

- 周期刷新 OLED

优先级：`Below Normal`

## I2C 资源共享问题

### 问题现象

在引入 FreeRTOS 后：

- OLED 显示随机异常
- 温湿度气压偶尔跳变
- 数据刷新不稳定

### 原因分析

BME280 与 SSD1306 共用：

```text
I2C1
```

多个任务同时访问 I2C 外设时，会导致总线竞争。

### 解决方案

使用 FreeRTOS Mutex：

```c
osMutexAcquire(I2CMutex, osWaitForever);

/* I2C 操作 */

osMutexRelease(I2CMutex);
```

确保任意时刻只有一个任务访问 I2C。

### 结果

连续运行测试：

- 无数据异常
- 无 OLED 花屏
- 无任务冲突

系统稳定运行。

## 开发过程中解决的问题

### BME280

- 湿度寄存器大小端问题
- `dig_H4` / `dig_H5` 校准参数解析
- 12 位补码符号扩展
- Bosch 湿度公式整数溢出
- Bosch 气压补偿算法实现

### FreeRTOS

- Task 创建与调度
- 任务优先级配置
- FreeRTOS Heap 使用
- I2C 资源竞争
- Mutex 同步机制

## 开发环境

- STM32CubeMX
- STM32 HAL Library
- FreeRTOS
- CLion
- CMake
- ARM GCC Toolchain

## 后续计划

### v1.1

- FreeRTOS Queue
- 任务间消息通信

### v1.2

- 软件定时器
- Event Group

### v1.3

- DMA 优化

### v2.0

- WiFi 数据上传
- MQTT
- Home Assistant 接入

## 学习收获

通过本项目掌握了：

- STM32 外设驱动开发
- I2C 通信
- 数据手册阅读
- Bosch 补偿算法实现
- FreeRTOS 多任务设计
- Mutex 资源同步
- 嵌入式项目分层架构设计

## 当前版本

v1.0

功能稳定运行：

- BME280
- SSD1306
- FreeRTOS
- Mutex
- 双任务架构
