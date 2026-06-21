#include "app_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_sensor.h"
#include "app_shared.h"
#include "FreeRTOS.h"
#include "app_logger.h"
#define FW_VERSION "v2.0"



void APP_ShellProcess(char *cmd)
{
    if(strcmp(cmd, "help") == 0)
    {
        printf("\r\n");
        printf("help\r\n");
        printf("version\r\n");
        printf("temp\r\n");
        printf("hum\r\n");
        printf("press\r\n");
        printf("stack\r\n");
        printf("all\r\n");
        printf("heap\r\n");
        printf("log\r\n");
        printf("clear\r\n");
    }
    else if(strcmp(cmd, "stack") == 0)
    {
        printf("\r\n");

        printf(
            "SensorTask  : %lu\r\n",
            osThreadGetStackSpace(
                SensorTaskHandle));

        printf(
            "DisplayTask : %lu\r\n",
            osThreadGetStackSpace(
                DisplayTaskHandle));

        printf(
            "ShellTask   : %lu\r\n",
            osThreadGetStackSpace(
                ShellTaskHandle));

        printf(
            "MonitorTask : %lu\r\n",
            osThreadGetStackSpace(
                MonitorTaskHandle));
    }
    else if(strcmp(cmd, "version") == 0)
    {
        printf("\r\n");
        printf("EnvMonitor %s\r\n", FW_VERSION);
    }
    else if(strcmp(cmd, "all") == 0)
    {
        printf("\r\n");

        printf(
            "T:%ld.%02ldC\r\n",
            g_latestData.temperature / 100,
            labs(g_latestData.temperature % 100));

        printf(
            "H:%ld.%02ld%%\r\n",
            g_latestData.humidity / 1024,
            (g_latestData.humidity % 1024)
            * 100 / 1024);

        printf(
            "P:%ld.%02ldhPa\r\n",
            g_latestData.pressure / 256 / 100,
            (g_latestData.pressure / 256) % 100);
    }
    else if(strcmp(cmd, "temp") == 0)
    {
        printf("\r\n");

        printf(
            "Temperature: %ld.%02ld C\r\n",
            g_latestData.temperature / 100,
            labs(g_latestData.temperature % 100));
    }
    else if(strcmp(cmd, "hum") == 0)
    {
        printf("\r\n");

        printf(
            "Humidity: %ld.%02ld %%\r\n",
            g_latestData.humidity / 1024,
            (g_latestData.humidity % 1024)
            * 100 / 1024);
    }
    else if(strcmp(cmd, "press") == 0)
    {
        printf("\r\n");

        printf(
            "Pressure: %ld.%02ld hPa\r\n",
            g_latestData.pressure / 256 / 100,
            (g_latestData.pressure / 256) % 100);
    }
    else if(strcmp(cmd, "heap") == 0)
    {
        printf("\r\n");

        printf(
            "Free Heap: %u Bytes\r\n",
            (unsigned int)xPortGetFreeHeapSize());
    }
    else if(strcmp(cmd, "log") == 0)
    {
        uint8_t count;

        printf("\r\n");

        count = Logger_GetCount();

        printf(
            "Log Count: %d\r\n\r\n",
            count);

        for(uint8_t i = 0;
            i < count;
            i++)
        {
            LoggerRecord_t *rec =
                Logger_GetRecord(i);

            printf(
                "[%lu ms] "
                "T:%ld.%02ld "
                "H:%ld.%02ld "
                "P:%ld.%02ld\r\n",

                rec->tick,

                rec->data.temperature / 100,
                labs(rec->data.temperature % 100),

                rec->data.humidity / 1024,
                (rec->data.humidity % 1024)
                    * 100 / 1024,

                rec->data.pressure / 256 / 100,
                (rec->data.pressure / 256)
                    % 100);
        }
    }
    else if(strcmp(cmd,"clear")==0)
    {
        Logger_Clear();

        printf(
            "Logger cleared\r\n");
    }
    else
    {
        printf("Unknown Command\r\n");
    }
}
