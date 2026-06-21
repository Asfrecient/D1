#ifndef APP_LOGGER_H
#define APP_LOGGER_H

#include "app_sensor.h"

#define LOGGER_MAX_RECORDS 16

typedef struct
{
    uint32_t tick;
    BME280_Data_t data;
} LoggerRecord_t;

void Logger_Record(BME280_Data_t *data);

uint8_t Logger_GetCount(void);

LoggerRecord_t* Logger_GetRecord(
    uint8_t index);

void Logger_Clear(void);

#endif