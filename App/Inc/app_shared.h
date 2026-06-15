#ifndef APP_SHARED_H
#define APP_SHARED_H

#include "cmsis_os2.h"


extern osMutexId_t I2CMutex;

extern osEventFlagsId_t DisplayEventHandle;

extern osMessageQueueId_t shellQueueHandle;

extern osThreadId_t SensorTaskHandle;
extern osThreadId_t DisplayTaskHandle;
extern osThreadId_t ShellTaskHandle;
extern osThreadId_t MonitorTaskHandle;

extern volatile uint8_t g_displayPage;



#define DISPLAY_EVENT_KEY 0x01

#endif