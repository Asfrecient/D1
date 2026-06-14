# AI_CONTEXT.md

# 项目简介

这是一个基于 STM32 + FreeRTOS 的嵌入式实习项目。

目标是逐步实现一个完整的环境监测与物联网系统。

当前已经完成：

* BME280 温湿度气压采集
* OLED 实时显示
* FreeRTOS 多任务架构
* Queue 任务间通信
* Mutex 保护 I2C 总线
* Software Timer + Semaphore 事件驱动采样

---

# 硬件环境

MCU：

* STM32F103C8T6

模块：

* OLED SSD1306（I2C）
* BME280（I2C）
* ESP8266 NodeMCU（未开发）
* W25Q64 Flash（未开发）
* 按键（未开发）
* 蜂鸣器（未开发）

开发工具：

* STM32CubeMX
* STM32CubeIDE
* CLion
* ST-Link

---

# 当前工程分层

工程采用 Driver + App + RTOS 分层设计。

```text
App
│
├── app_sensor.c
├── app_sensor.h
│
├── app_display.c
├── app_display.h
│
Drivers
│
├── bme280.c
├── bme280.h
│
├── oled.c
├── oled.h
│
FreeRTOS
│
└── freertos.c
```

---

# 各层职责

## Driver层

负责：

* 寄存器读写
* I2C通信
* OLED驱动
* BME280驱动

特点：

* 不依赖FreeRTOS
* 不包含业务逻辑

---

## App层

负责：

* 业务逻辑
* 数据处理
* OLED显示格式化

主要文件：

```text
app_sensor.c
app_sensor.h

app_display.c
app_display.h
```

---

## RTOS层

负责：

* 创建任务
* Queue
* Mutex
* Timer
* Semaphore

主要文件：

```text
freertos.c
```

---

# BME280状态

已经完成：

* 温度补偿算法
* 湿度补偿算法
* 气压补偿算法

当前输出正常。

示例：

```text
T:25.34C

H:48.21%

P:1008.01hPa
```

已经处理：

* 温度负数显示
* 气压显示格式
* OLED布局优化

---

# OLED状态

工作正常。

当前显示内容：

```text
T:25.34C

H:48.21%

P:1008.01hPa
```

显示接口：

```c
APP_DisplayUpdate(...)
```

---

# FreeRTOS状态

当前已经使用：

## Task

```text
SensorTask
DisplayTask
```

---

## Queue

```text
sensorQueueHandle
```

作用：

SensorTask：

```text
读取BME280
↓
发送Queue
```

DisplayTask：

```text
接收Queue
↓
刷新OLED
```

---

## Mutex

```text
I2CMutexHandle
```

原因：

OLED和BME280共用I2C总线。

曾出现：

* OLED显示乱跳
* 传感器数据异常

最终使用Mutex解决。

规则：

所有I2C访问必须先获取Mutex。

---

## Software Timer

```text
SensorTimerHandle
```

周期：

```text
1000ms
```

作用：

定时触发传感器采样。

---

## Semaphore

```text
SensorSemHandle
```

作用：

Timer通知SensorTask开始采样。

---

# 当前RTOS架构

```text
SensorTimer
      │
      ▼
SensorSem
      │
      ▼
SensorTask
      │
      ▼
sensorQueue
      │
      ▼
DisplayTask
      │
      ▼
OLED
```

---

# Queue数据结构

```c
typedef struct
{
    int32_t temperature;
    uint32_t humidity;
    uint32_t pressure;
} BME280_Data_t;
```

特点：

* 使用Queue传递数据
* 已删除全局sensor变量
* 不允许重新引入全局sensor共享数据

---

# Timer + Semaphore设计

当前为事件驱动架构。

Timer回调：

```c
osSemaphoreRelease(
    SensorSemHandle);
```

SensorTask：

```c
osSemaphoreAcquire(
    SensorSemHandle,
    osWaitForever);
```

特点：

* 不使用轮询
* 不使用osDelay(1000)定时采样
* 由Timer触发任务运行

---

# 已解决的问题

## 问题1

OLED与BME280同时访问I2C。

现象：

```text
数据显示乱跳
```

解决：

```text
I2C Mutex
```

---

## 问题2

DisplayTask只执行一次。

现象：

```text
Queue Put持续输出
DisplayTask没有继续运行
```

原因：

```text
Task栈不足
```

解决：

```text
增大Task Stack
```

---

## 问题3

FreeRTOS Heap不足。

默认：

```c
configTOTAL_HEAP_SIZE = 3072
```

导致：

```text
任务创建失败
Queue异常
```

解决：

```c
configTOTAL_HEAP_SIZE = 8192
```

---

# FreeRTOS配置

当前：

```c
configTOTAL_HEAP_SIZE = 8192
```

不要降低。

原因：

当前已经使用：

* Queue
* Mutex
* Semaphore
* Timer
* 多任务

---

# CubeMX开发规则

必须遵守：

不要把业务逻辑直接写在：

```text
main.c
```

或者：

```text
freertos.c
```

中。

业务代码应放在：

```text
App层
```

---

所有CubeMX管理文件中：

只允许在：

```c
/* USER CODE BEGIN */

/* USER CODE END */
```

之间修改代码。

避免Generate Code时被覆盖。

---

# 编码规范

推荐：

```c
APP_SensorRead(...)
APP_DisplayUpdate(...)
```

不推荐：

```c
Task直接调用Driver
```

原则：

```text
Task
 ↓
App
 ↓
Driver
```

---

# 当前版本

已完成：

## v1.0

* BME280驱动
* OLED驱动

---

## v1.1

* Queue通信

---

## v1.2

* App层重构

---

## v1.3

* Software Timer
* Binary Semaphore
* 事件驱动采样

当前稳定版本：

```text
v1.3
```

---

# 后续规划

## v1.4

MonitorTask

学习：

* 任务监控
* 栈监控

---

## v1.5

按键 + Event Flags

学习：

* Event Flags
* 多事件同步

---

## v1.6

蜂鸣器报警

---

## v2.0

UART Shell

支持：

```text
temp
hum
press
all
```

命令查询

---

## v2.1

Logger模块

---

## v2.2

W25Q64数据记录

实现：

```text
历史数据存储
```

---

## v3.0

ESP8266驱动

---

## v3.1

MQTT

---

## v3.2

云平台显示

---

# AI协作注意事项

修改代码前必须遵守：

1. 保持当前分层架构。
2. 不允许恢复全局sensor变量。
3. 保持Queue通信方式。
4. 保持I2C Mutex保护。
5. Driver层不得依赖FreeRTOS。
6. App层负责业务逻辑。
7. FreeRTOS API使用CMSIS-RTOS2。
8. MCU为STM32F103C8T6。
9. 所有BME280数据均采用定点数表示。
10. 所有新增功能优先放入App层实现。
