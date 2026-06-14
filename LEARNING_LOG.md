# LEARNING_LOG.md

# STM32嵌入式实习项目学习日志

记录项目开发过程中遇到的问题、分析过程和最终解决方案。

目的：

* 避免重复踩坑
* 帮助AI快速理解项目历史
* 形成个人经验积累

---

# 阶段一：BME280驱动开发

---

## 问题1：温度正常，气压始终异常

现象：

```text
温度正常
湿度正常
气压明显错误
```

怀疑：

```text
I2C读取错误
校准参数错误
Bosch补偿公式错误
```

最终定位：

```text
BME280气压补偿公式未完整实现
```

解决：

严格按照Bosch官方补偿算法实现：

```c
var1
var2
p
```

全部使用：

```c
int64_t
```

不能使用：

```c
int32_t
```

否则中间计算溢出。

经验：

```text
BME280气压补偿必须使用64位整数。
```

---

## 问题2：不知道为什么要用BME280_ReadRawPressure()

当时误解：

```text
已经有气压值
为什么还要读RawPressure
```

理解后：

```text
RawPressure是ADC原始值

补偿算法需要：

adc_P
+
校准参数

才能计算真实气压
```

经验：

```text
Raw数据不是最终数据。

BME280所有环境参数都必须经过补偿计算。
```

---

## 问题3：气压单位混乱

Bosch输出：

```text
Pa × 256
```

OLED直接显示：

```text
258000000
```

明显错误。

解决：

显示前：

```c
pressure / 256
```

得到：

```text
Pa
```

继续：

```c
Pa / 100
```

得到：

```text
hPa
```

最终显示：

```text
1008.01hPa
```

经验：

```text
先搞清内部单位，再做显示。
```

---

# 阶段二：OLED显示优化

---

## 问题4：负温度显示异常

原代码：

```c
temperature % 100
```

温度为负时：

```text
-5.34℃
```

显示错误。

解决：

```c
labs(
    temperature % 100
)
```

经验：

```text
定点数显示必须考虑负数余数问题。
```

---

## 问题5：OLED布局不断调整

经历：

```text
Pa显示过长
温度格式不统一
湿度显示不整齐
```

最终格式：

```text
T:25.34C

H:48.21%

P:1008.01hPa
```

经验：

```text
显示格式属于业务逻辑。

应放在App层。
```

---

# 阶段三：FreeRTOS初次接入

---

## 问题6：CubeMX生成代码覆盖业务代码

最初：

```text
直接在main.c写业务代码
```

后果：

```text
Generate Code后被覆盖
```

解决：

建立：

```text
App层
```

新增：

```text
app_sensor.c
app_sensor.h

app_display.c
app_display.h
```

经验：

```text
业务逻辑不要直接写在CubeMX管理文件。
```

---

## 问题7：不知道代码该放哪里

逐步形成：

```text
Driver层
App层
RTOS层
```

职责划分：

Driver：

```text
寄存器
I2C
硬件访问
```

App：

```text
业务逻辑
数据显示
```

RTOS：

```text
任务
队列
同步
```

经验：

```text
Task → App → Driver
```

---

# 阶段四：OLED数据乱跳

---

## 问题8：OLED显示一段时间后数值乱跳

现象：

```text
显示偶尔异常
数值突然变化
```

排查：

发现：

```text
OLED
BME280
共用I2C
```

同时访问。

解决：

增加：

```text
I2C Mutex
```

所有I2C访问：

```c
osMutexAcquire()
...
osMutexRelease()
```

经验：

```text
共享总线必须加Mutex。
```

---

# 阶段五：Queue重构

---

## 问题9：使用全局sensor变量

早期方案：

```c
extern BME280_Data_t sensor;
```

问题：

```text
任务之间直接共享数据
```

后期扩展风险大。

解决：

使用：

```text
Message Queue
```

SensorTask：

```text
Put
```

DisplayTask：

```text
Get
```

经验：

```text
任务之间优先使用Queue通信。
```

---

## 问题10：不理解Queue

后来理解：

SensorTask：

```text
生产者
```

DisplayTask：

```text
消费者
```

模型：

```text
Producer
↓
Queue
↓
Consumer
```

经验：

```text
Queue不仅是数据存储，
更是任务解耦工具。
```

---

# 阶段六：DisplayTask只执行一次

---

## 问题11：Queue Put持续输出

现象：

```text
Queue Put
Queue Put
Queue Put
...
```

但是：

```text
DisplayTask只执行一次
```

最初怀疑：

```text
Queue坏了
Queue满了
代码逻辑错误
```

最终原因：

```text
Task Stack太小
```

解决：

增大：

```text
DisplayTask Stack
```

经验：

```text
RTOS问题先怀疑：
1. Stack
2. Heap
3. 同步机制
```

---

# 阶段七：FreeRTOS Heap不足

---

## 问题12：各种奇怪现象

现象：

```text
任务创建失败
对象创建失败
运行异常
```

最终发现：

```c
configTOTAL_HEAP_SIZE
```

默认：

```text
3072
```

太小。

解决：

改为：

```text
8192
```

经验：

```text
Queue
Mutex
Semaphore
Timer
Task

都会消耗Heap。
```

---

# 阶段八：Timer + Semaphore

---

## 问题13：不理解Semaphore

最初疑问：

```text
为什么不用osDelay()
```

后来理解：

旧方案：

```text
Task自己定时
```

新方案：

```text
Timer
↓
Semaphore
↓
Task
```

本质：

```text
Release
=
发通知

Acquire
=
等通知
```

经验：

```text
Semaphore主要用于同步。

Mutex主要用于资源保护。
```

---

## 问题14：事件驱动思想

以前：

```c
while(1)
{
    工作
    延时
}
```

现在：

```text
收到事件
↓
开始工作
```

理解：

```text
RTOS核心思想：
事件驱动
而不是轮询。
```

---

# 当前形成的开发原则

---

## 原则1

```text
Task不要直接操作硬件。
```

推荐：

```text
Task
↓
App
↓
Driver
```

---

## 原则2

```text
共享资源必须加Mutex。
```

例如：

```text
I2C
SPI
UART
```

---

## 原则3

```text
任务之间优先Queue通信。
```

避免：

```text
全局变量共享
```

---

## 原则4

```text
CubeMX管理文件尽量少改。
```

业务代码放：

```text
App层
```

---

## 原则5

```text
出现异常先查：

Stack
Heap
Mutex
Queue
```

不要第一时间怀疑驱动。

---

# 当前项目状态

已完成：

```text
v1.0 BME280 + OLED

v1.1 Queue

v1.2 App层重构

v1.3 Timer + Semaphore
```

下一阶段：

```text
v1.4 MonitorTask

v1.5 Event Flags

v1.6 Buzzer

v2.0 UART Shell
```

---

# 最重要的收获

从：

```text
单片机裸机思维
```

逐步转变为：

```text
RTOS思维

任务
同步
通信
分层
事件驱动
```

这是本项目目前最大的学习成果。
