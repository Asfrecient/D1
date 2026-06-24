#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

typedef struct
{
    uint32_t sampleIntervalMs;
} AppConfig_t;

void Config_Init(void);

AppConfig_t *Config_Get(void);

void Config_SetSampleInterval(
    uint32_t interval);

#endif