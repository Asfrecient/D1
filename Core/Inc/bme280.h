#ifndef __BME280_H
#define __BME280_H

#include "main.h"

void BME280_Init(void);
int32_t BME280_ReadTemperature(void);
int32_t BME280_ReadHumidity(void);
int32_t BME280_ReadPressure(void);

#endif