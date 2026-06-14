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

## 当前RTOS对象

Task：

```text
SensorTask
DisplayTask
MonitorTask
```

Queue：

```text
sensorQueueHandle
```

Mutex：

```text
I2CMutexHandle
```

Semaphore：

```text
SensorSemHandle
```

Software Timer：

```text
SensorTimerHandle
```

当前工程已经使用：

```text
Task
Queue
Mutex
Semaphore
Software Timer
```

尚未使用：

```text
Event Flags
Message Buffer
Stream Buffer
```

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

MonitorTask
      │
      ▼
每5秒打印任务剩余栈空间
```

## 当前任务优先级

```text
SensorTask   Normal

DisplayTask  BelowNormal

MonitorTask  Low
```

设计原则：

```text
采样优先于显示

显示优先于调试输出
```

MonitorTask属于后台诊断任务。

即使MonitorTask出现异常，也不应影响：

* 传感器采样
* OLED显示


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

## 问题4

Task Stack不足时，任务可能不是明显报错，而是表现为只执行一次或后续不再运行。

当前增加：

```text
MonitorTask
```

用于周期打印：

```text
SensorTask剩余栈空间
DisplayTask剩余栈空间
```

经验：

* 调试RTOS异常时优先观察Stack和Heap
* 不要随意降低Task Stack
* 不要降低 `configTOTAL_HEAP_SIZE = 8192`

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
* MonitorTask栈监控

当前任务栈配置：

SensorTask：

```c
.stack_size = 512 * 4
```

即：

```text
2048 Bytes
```

DisplayTask：

```c
.stack_size = 512 * 4
```

即：

```text
2048 Bytes
```

MonitorTask：

```c
.stack_size = 256 * 4
```

即：

```text
1024 Bytes
```

MonitorTask实测输出：

```text
SensorTask  Stack : 1464
DisplayTask Stack : 1536
```

估算：

```text
SensorTask  已使用约584 Bytes

DisplayTask 已使用约512 Bytes
```


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

* Software Timer + Semaphore事件驱动采样

---

## v1.4

* MonitorTask任务栈监控

---

# 当前稳定版本

```text
v1.4
```

---

# 当前技术债

# 当前技术债

目标架构：

```text
Task
 ↓
App
 ↓
Driver
```

原则上：

```text
Driver层不依赖FreeRTOS
```

当前实际情况：

OLED与BME280共用I2C1。

为了避免总线竞争，

```c
BME280_ReadData(...)
```

内部直接使用：

```c
osMutexAcquire(...)
osMutexRelease(...)
```

因此：

```text
当前BME280驱动已经依赖FreeRTOS Mutex
```

这是学习阶段的简化实现。

后续可以考虑：

```text
App层统一加锁

或

封装I2C访问层
```

从而恢复：

```text
Driver层不依赖RTOS
```

的目标架构。


---

# 后续规划

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
11. SensorTask必须保持事件驱动模式。

当前正确架构：

```text
Software Timer
      ↓
Semaphore
      ↓
SensorTask
```

不要改回：

```c
for (;;)
{
    BME280_ReadData();
    osDelay(...);
}
```

除非明确进行架构调整。

12. MonitorTask属于调试工具。

允许增加：

* Heap监控
* Queue监控
* Runtime统计

不要让MonitorTask参与业务逻辑。

13. 当前任务优先级设计：

```text
SensorTask   Normal

DisplayTask  BelowNormal

MonitorTask  Low
```

新增任务时应明确说明优先级设计原因。

