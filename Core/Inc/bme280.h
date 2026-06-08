#ifndef __BME280_H
#define __BME280_H

#include "main.h"

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
} BME280_Calib_t;

extern BME280_Calib_t bme280_calib;



uint8_t BME280_ReadID(void);

uint8_t BME280_ReadReg(uint8_t reg);

int32_t BME280_ReadRawTemp(void);

uint16_t BME280_ReadU16(uint8_t reg);

void BME280_WriteReg(uint8_t reg,uint8_t data);

void BME280_DumpCalibration(void);

void BME280_ReadCalibration(void);

#endif