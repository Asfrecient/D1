#include "app_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_config.h"
#include "app_sensor.h"
#include "app_shared.h"
#include "FreeRTOS.h"
#include "app_logger.h"
#include "app_storage.h"
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
        printf("show config\r\n");
        printf("set interval <ms>\r\n");
        printf("save\r\n");

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
    else if(strcmp(cmd, "show config") == 0||
        strcmp(cmd, "config") == 0)
    {
        AppConfig_t *cfg = Config_Get();

        printf("\r\n");
        printf("===== CONFIG =====\r\n");
        printf("Sample Interval : %lu ms\r\n",
               cfg->sampleIntervalMs);
        printf("==================\r\n");
    }
    else if(strncmp(cmd,
                "set interval ",
                13) == 0)
    {
        uint32_t interval =
    atoi(&cmd[13]);

        if(interval == 0)
        {
            printf(
                "Usage: set interval <ms>\r\n");
        }
        else if(interval < 100 ||
                interval > 60000)
        {
            printf(
                "Range: 100~60000 ms\r\n");
        }
        else
        {
            Config_SetSampleInterval(
                interval);

            printf(
                "Interval = %lu ms\r\n",
                interval);
        }
    }

    else if(strcmp(cmd,
               "save") == 0)
    {
        Storage_SaveConfig(
            Config_Get());

        printf(
            "Config Saved\r\n");
    }

    else
    {
        printf("Unknown Command\r\n");
    }
}
