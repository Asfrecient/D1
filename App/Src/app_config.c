#include "app_config.h"

#include "cmsis_os2.h"

static AppConfig_t g_config;

void Config_Init(void)
{
    g_config.sampleIntervalMs = 1000;
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