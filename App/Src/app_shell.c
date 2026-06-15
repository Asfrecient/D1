#include "app_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_sensor.h"
#include "app_shared.h"
#include "FreeRTOS.h"
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
    else
    {
        printf("Unknown Command\r\n");
    }
}
