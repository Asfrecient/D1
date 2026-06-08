#ifndef __BME280_H
#define __BME280_H

#include "main.h"

uint8_t BME280_ReadID(void);

uint8_t BME280_ReadReg(uint8_t reg);

int32_t BME280_ReadRawTemp(void);

void BME280_WriteReg(uint8_t reg,uint8_t data);

void BME280_DumpCalibration(void);

#endif