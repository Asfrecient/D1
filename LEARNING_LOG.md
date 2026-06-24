# LEARNING_LOG.md

# STM32嵌入式实习项目学习日志

记录项目开发过程中遇到的问题、分析过程和最终解决方案。

目的：

* 避免重复踩坑
* 帮助AI快速理解项目历史
* 形成个人经验积累

---

# 阶段十：v2.2 Config System

---

## 问题30：为什么不能把采样周期写死在代码里

最初写法：

```c
osTimerStart(..., 1000);
```

新的理解：

```text
采样周期属于运行时配置，
不应该散落在业务代码里。
```

改为：

```text
Config_Get()->sampleIntervalMs
```

经验：

```text
配置和业务逻辑要分离。
```

---

## 问题31：Config 模块为什么要单独封装

新增文件：

```text
app_config.h
app_config.c
```

职责：

```text
保存配置
读取配置
修改配置
```

理解：

```text
Shell 不直接改底层资源，
而是通过 Config 模块统一调度。
```

经验：

```text
统一入口能让后续扩展更稳。
```

---

## 问题32：软件定时器为什么适合控制采样

当前链路：

```text
SensorTimer
↓
Semaphore
↓
SensorTask
↓
APP_SensorRead()
```

理解：

```text
Timer 负责周期，
Task 负责执行。
```

经验：

```text
不要用任务自己死等周期。
```

---

## 问题33：怎么让修改采样周期立即生效

新增接口：

```c
Config_SetSampleInterval()
```

动作：

```text
停止 SensorTimer
使用新周期重新启动 SensorTimer
```

效果：

```text
修改后无需重启系统，
新周期立即生效。
```

经验：

```text
运行时配置变化
应该直接反映到调度层。
```

---

## 问题34：Shell 为什么要能控制系统行为

新增命令：

```text
show config
config
set interval
```

理解：

```text
Shell 不只是只读调试口，
也可以变成配置入口。
```

经验：

```text
CLI 一旦能写配置，
系统就开始有产品感了。
```

---

## 问题35：参数输入为什么必须校验

范围限制：

```text
100 ~ 60000 ms
```

非法输入：

```text
set interval abc
```

处理：

```text
Usage: set interval <ms>
```

经验：

```text
所有用户输入都不能直接信任。
```

---

## 问题36：atoi() 为什么不够安全

问题：

```text
atoi("abc")
```

结果：

```text
0
```

风险：

```text
非法输入可能被误当成合法配置。
```

经验：

```text
需要 Usage 检查和范围检查配合使用。
```

---

## 问题37：UART 中断里为什么会丢字符

现象：

```text
show config
↓
show confi
```

原因：

```text
重新开启接收太晚。
```

修复：

```text
先保存接收字符
立即调用 HAL_UART_Receive_IT(...)
再处理后续逻辑
```

经验：

```text
ISR 要尽快把接收链路续上。
```

---

## 问题38：为什么要梳理整个系统架构

当前链路：

```text
UART IRQ
↓
Shell Queue
↓
ShellTask
↓
Config Module
↓
SensorTimer
↓
SensorTask
↓
Logger
↓
Ring Buffer
↓
sensorQueue
↓
DisplayTask
```

理解：

```text
配置入口
已经能影响采样、日志和显示链路。
```

经验：

```text
当模块变多时，
先画出系统流向再继续扩展。
```

---

## 当前掌握内容

已掌握：

```text
FreeRTOS Task
Queue
Semaphore
Mutex
EventFlags
Software Timer
UART Interrupt
Shell Framework
Logger System
Ring Buffer
Config System
```

下一阶段：

```text
v3.0 Persistent Storage
```

目标：

```text
W25Q64
Flash存储
配置持久化
Factory Reset
参数恢复
```

---

# 阶段九：v2.1 Logger System

---

## 问题22：Shell 命令解析和中断接收耦合太紧

最初理解：

```text
USART 中断收到什么
就立刻在中断里处理什么
```

后来调整：

```text
USART1 中断
↓
Message Queue
↓
ShellTask
↓
命令解析
```

经验：

```text
中断只做接收和投递，
复杂逻辑放到任务里处理。
```

---

## 问题23：为什么要引入 Logger System

最初疑问：

```text
已经有 Shell 了，
为什么还要再做 Logger
```

理解后：

```text
Shell 负责交互
Logger 负责历史记录
```

作用：

```text
保存采样历史
支持 log 查询
支持 clear 清空
为后续故障分析做准备
```

经验：

```text
调试输出不等于历史记录，
Logger 才是可回溯数据。
```

---

## 问题24：Ring Buffer 怎么保存固定容量历史

核心结构：

```c
typedef struct
{
    uint32_t tick;
    BME280_Data_t data;
} LoggerRecord_t;
```

关键变量：

```c
g_logBuffer[]
g_logWriteIndex
g_logCount
```

理解：

```text
固定数组循环使用
写满后覆盖最旧数据
```

经验：

```text
Ring Buffer 是日志系统最常见的数据结构之一。
```

---

## 问题25：不知道怎么按时间顺序读取历史记录

核心代码：

```c
realIndex =
    (g_logWriteIndex + index)
    % LOGGER_MAX_RECORDS;
```

理解：

```text
index
= 第几条历史记录

realIndex
= 数组真实下标
```

效果：

```text
最旧 -> 最新
```

经验：

```text
模运算是环形缓冲区的关键。
```

---

## 问题26：log 命令为什么要单独实现

新增命令：

```text
log
clear
```

功能：

```text
log
= 显示历史采样

clear
= 清空历史记录
```

输出字段：

```text
Tick
Temperature
Humidity
Pressure
```

经验：

```text
Shell 的价值不只是调试，
也可以逐步演化成产品级 CLI。
```

---

## 问题27：Tick 时间戳有什么意义

理解：

```text
osKernelGetTickCount()
```

用途：

```text
日志记录
超时判断
性能统计
```

经验：

```text
Tick 是 RTOS 最基础的时间基准。
```

---

## 问题28：Logger 和 Queue 的关系

当前链路：

```text
SensorTask
↓
Logger_Record()
↓
Ring Buffer
```

同时保留：

```text
SensorTask
↓
sensorQueue
↓
DisplayTask
```

理解：

```text
Queue 负责任务解耦
Logger 负责历史保存
```

经验：

```text
数据传递和数据留存
是两件不同的事。
```

---

## 问题29：clear 命令为什么不仅是删数据

实现：

```c
g_logCount = 0;
g_logWriteIndex = 0;
```

效果：

```text
清空日志记录
恢复空缓冲区状态
```

经验：

```text
清空操作也要同步重置索引，
否则 Ring Buffer 状态会错乱。
```

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

过小。

解决：

```text
增大FreeRTOS Heap
```

经验：

```text
对象创建失败或行为异常时，要检查Heap是否足够。
```

---

# 阶段八：v1.5 按键与页面切换

---

## 问题13：EXTI中断完整链路不清楚

完整链路：

```text
按键
↓
EXTI
↓
NVIC
↓
IRQHandler
↓
HAL_GPIO_EXTI_IRQHandler
↓
HAL_GPIO_EXTI_Callback
↓
用户代码
```

理解：

```text
HAL_GPIO_EXTI_Callback()
本质是HAL库提供的中断回调函数
```

经验：

```text
先搞清中断完整调用链，再写用户逻辑。
```

---

## 问题14：Event Flags本质不理解

理解后：

```text
Event Flags本质上是一个32位状态寄存器
```

例如：

```c
#define DISPLAY_EVENT_KEY 0x01
```

对应：

```text
bit0
```

以后可以扩展：

```text
0x01
0x02
0x04
0x08
...
```

分别对应不同事件。

经验：

```text
Event Flags适合表达“多个离散事件”的通知状态。
```

---

## 问题15：位运算检查事件

代码：

```c
if(flags & DISPLAY_EVENT_KEY)
```

本质：

```text
检查bit0是否为1
```

与BME280中的：

```c
if(value & 0x0800)
```

属于同一种位操作思想。

经验：

```text
位掩码判断是嵌入式开发中的通用模式。
```

---

## 问题16：osEventFlagsClear理解

代码：

```c
osEventFlagsClear(
    DisplayEventHandle,
    DISPLAY_EVENT_KEY);
```

含义：

```text
在DisplayEventHandle中
清除DISPLAY_EVENT_KEY对应的bit
```

本质类似：

```c
flags &= ~mask;
```

经验：

```text
Set / Check / Clear 要作为一套事件状态管理来理解。
```

---

## 问题17：^= 运算符用于页面切换

代码：

```c
g_displayPage ^= 1;
```

效果：

```text
0 → 1
1 → 0
```

用于页面切换。

经验：

```text
异或翻转非常适合双状态切换。
```

---

## 问题18：机械按键抖动导致重复触发

实际波形：

```text
按下
↓
多次抖动
↓
稳定
```

导致：

```text
一次按压
触发多次中断
```

解决：

```c
HAL_GetTick()

if(now - g_lastKeyTick < 50)
{
    return;
}
```

实现：

```text
50ms软件消抖
```

经验：

```text
机械按键默认要考虑消抖，软件消抖是最直接方案。
```

---

## 问题19：return的作用需要重新理解

代码：

```c
if(now - g_lastKeyTick < 50)
{
    return;
}
```

含义：

```text
立即退出当前函数
```

属于：

```text
Early Return
（提前返回）
```

常用于：

```text
参数检查
错误处理
消抖
状态过滤
```

经验：

```text
Early Return能让中断回调和状态过滤逻辑更清晰。
```

---

## 问题20：RTOS任务响应速度为什么慢

问题：

```text
KEY立即打印
Page切换延迟
```

原因：

```c
osMessageQueueGet(
    ...,
    osWaitForever);
```

导致：

```text
DisplayTask一直阻塞
```

结论：

```text
任务响应速度
不取决于CPU速度

而取决于任务什么时候被唤醒
```

经验：

```text
RTOS性能分析要先看阻塞点和唤醒条件。
```

---

## 问题21：OLED刷新闪屏

问题：

```text
每次刷新都OLED_Clear()
```

现象：

```text
闪屏
```

解决：

```text
页面切换时清屏
数据刷新时不清屏
```

经验：

```text
页面切换 → Clear

页面刷新 → 局部更新
```

这是嵌入式UI开发常见优化思路。

---

# 阶段总结

v1.5重点掌握：

```text
EXTI
NVIC
IRQHandler
HAL_GPIO_EXTI_Callback
Event Flags
位运算
软件消抖
UI页面切换
RTOS响应分析
OLED刷新优化
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

# 阶段九：MonitorTask 与任务栈监控

---

## 问题15：任务栈大小到底应该设置多少？

之前配置任务时：

```c
.stack_size = 512 * 4
```

能够运行，但并不知道：

```text
实际用了多少？
是否浪费？
是否存在栈溢出风险？
```

属于经验配置。

---

## 尝试方案

新增：

```text
MonitorTask
```

作用：

```text
周期监控任务运行状态
```

MonitorTask优先级：

```text
Low
```

避免影响正常业务任务。

---

## 学习的新API

CMSIS-RTOS2提供：

```c
osThreadGetStackSpace()
```

作用：

```text
获取任务剩余栈空间
```

使用示例：

```c
osThreadGetStackSpace(
    SensorTaskHandle);

osThreadGetStackSpace(
    DisplayTaskHandle);
```

---

## 实际测试结果

当前配置：

```c
.stack_size = 512 * 4
```

即：

```text
2048 Bytes
```

MonitorTask输出：

```text
===== Task Monitor =====

SensorTask  Stack : 1464

DisplayTask Stack : 1536

========================
```

---

## 数据分析

SensorTask：

```text
总栈：
2048 Bytes

剩余：
1464 Bytes

已使用：
约584 Bytes
```

---

DisplayTask：

```text
总栈：
2048 Bytes

剩余：
1536 Bytes

已使用：
约512 Bytes
```

---

## 重要发现

DisplayTask实际栈消耗比预期更高。

原因可能包括：

```c
char str[40];

sprintf(...);
sprintf(...);
sprintf(...);
```

其中：

```c
sprintf()
```

属于栈消耗较大的函数。

---

## 与历史问题关联

此前曾出现：

```text
DisplayTask只执行一次
```

当时通过增大Stack后恢复正常。

v1.4阶段通过运行时监控验证：

```text
DisplayTask确实存在较高栈消耗
```

说明之前的问题极有可能与栈不足有关。

---

## MonitorTask的价值

以前排查问题时：

```text
怀疑Queue
怀疑驱动
怀疑逻辑
```

缺乏客观依据。

现在：

```text
可以直接观察Stack剩余量
```

判断：

```text
是否接近栈溢出
是否需要调整配置
```

---

## 学到的新概念

### Runtime Diagnostics（运行时诊断）

不是只看代码。

而是在程序运行过程中实时观察：

```text
Stack
Heap
Task状态
Queue状态
```

定位问题。

MonitorTask属于：

```text
运行时诊断工具
```

---

## 经验总结

经验1：

```text
Task Stack不要靠猜。
```

应通过实际运行数据评估。

---

经验2：

```text
出现RTOS异常时先查资源。
```

排查顺序：

```text
1. Stack
2. Heap
3. Queue
4. Mutex
5. Semaphore
6. 业务逻辑
```

---

经验3：

```text
sprintf()
是典型的大栈消耗函数。
```

在资源受限系统中需要特别注意。

---

经验4：

```text
MonitorTask应长期保留。
```

后续开发：

```text
UART Shell
W25Q64
ESP8266
MQTT
```

时仍然可以用于定位问题。

---

## 本阶段收获

从：

```text
给任务随便分配栈大小
```

转变为：

```text
通过运行时数据评估任务资源占用
```

开始具备：

```text
RTOS资源分析能力
```

这是从“会用FreeRTOS”到“会调试FreeRTOS”的重要一步。

---

## 当前版本

完成：

```text
v1.4 MonitorTask
```

掌握：

```text
osThreadGetStackSpace()

任务栈监控

运行时诊断
```

下一阶段：

```text
v1.5 Event Flags + 按键输入
```
