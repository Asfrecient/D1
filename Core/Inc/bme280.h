#ifndef __BME280_H
#define __BME280_H

#include "main.h"


typedef struct
{
 int32_t temperature;
 int32_t humidity;
 int32_t pressure;
}BME280_Data_t;

void BME280_Init(void);
int32_t BME280_ReadTemperature(void);
int32_t BME280_ReadHumidity(void);
int32_t BME280_ReadPressure(void);

void BME280_ReadData(BME280_Data_t *data);

#endif