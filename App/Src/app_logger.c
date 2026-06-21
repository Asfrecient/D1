#include "app_logger.h"
#include "cmsis_os.h"

//真正存数据的地方
static LoggerRecord_t g_logBuffer[LOGGER_MAX_RECORDS];
//下一次写哪里
static uint8_t g_logWriteIndex = 0;
//当前已经存了多少条
static uint8_t g_logCount = 0;

void Logger_Record(BME280_Data_t *data)
{

    g_logBuffer[g_logWriteIndex].tick =
        osKernelGetTickCount();

    g_logBuffer[g_logWriteIndex].data = *data;

    g_logWriteIndex++;

    if(g_logWriteIndex >= LOGGER_MAX_RECORDS)
    {
        g_logWriteIndex = 0;
    }

    if(g_logCount < LOGGER_MAX_RECORDS)         //当缓冲区满了以后,g_logCount不在增加,例如MAX = 4,后面都是4
    {
        g_logCount++;
    }
}

uint8_t Logger_GetCount(void)
{
    return g_logCount;
}

LoggerRecord_t* Logger_GetRecord(
    uint8_t index)
{
    uint8_t realIndex;

    if(index >= g_logCount)
    {
        return NULL;
    }

    if(g_logCount < LOGGER_MAX_RECORDS)
    {
        realIndex = index;
    }
    else
    {
        realIndex =
            (g_logWriteIndex + index)
            % LOGGER_MAX_RECORDS;
    }

    return &g_logBuffer[realIndex];
}

void Logger_Clear(void)
{
    g_logCount = 0;
    g_logWriteIndex = 0;
}