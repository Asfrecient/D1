#include "app_config.h"

#include <stdbool.h>
#include <stdio.h>

#include "app_storage.h"
#include "cmsis_os2.h"

static AppConfig_t g_config;

void Config_Init(void)
{
    if(Storage_LoadConfig(
            &g_config))
    {
        printf(
          "Config Loaded\r\n");
    }
    else
    {
        g_config.sampleIntervalMs =
            1000;

        printf(
          "Default Config\r\n");
    }
}

AppConfig_t *Config_Get(void)
{
    return &g_config;
}

extern osTimerId_t SensorTimerHandle;

void Config_SetSampleInterval(
    uint32_t interval)
{
    g_config.sampleIntervalMs =
        interval;

    osTimerStop(
        SensorTimerHandle);

    osTimerStart(
        SensorTimerHandle,
        interval);
}