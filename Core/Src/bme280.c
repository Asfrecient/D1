#include "bme280.h"

#include <stdio.h>

#include "i2c.h"

#define BME280_ADDR (0x76 << 1)

BME280_Calib_t bme280_calib;

void BME280_ReadCalibration(void)
{
    bme280_calib.dig_T1 =
        BME280_ReadU16(0x88) ;


    bme280_calib.dig_T2 =
        (int16_t)BME280_ReadU16(0x8A) ;


    bme280_calib.dig_T3 =
        (int16_t)BME280_ReadU16(0x8C) ;

}

//读寄存器
uint8_t BME280_ReadReg(uint8_t reg)
{
    uint8_t data = 0;

    HAL_I2C_Mem_Read(
            &hi2c1,
            BME280_ADDR,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            &data,
            1,
            100);

    return data;
}
uint16_t BME280_ReadU16(uint8_t reg) {
    uint8_t lsb;
    uint8_t msb;
    lsb = BME280_ReadReg(reg);
    msb = BME280_ReadReg(reg + 1);
    return ((uint16_t)msb << 8) | lsb;
}


//写寄存器
void BME280_WriteReg(uint8_t reg,uint8_t data)
{
    HAL_I2C_Mem_Write(
            &hi2c1,
            BME280_ADDR,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            &data,
            1,
            100);
}

//读ID
uint8_t BME280_ReadID(void)
{
    return BME280_ReadReg(0xD0);
}

void BME280_DumpCalibration(void)
{
    for(uint8_t reg = 0x88;
        reg <= 0xA1;
        reg++)
    {
        printf("0x%02X = 0x%02X\r\n",
                reg,
                BME280_ReadReg(reg));
    }
}

int32_t BME280_ReadRawTemp(void)
{

    uint8_t data[3];
    int32_t adc_T;
    HAL_I2C_Mem_Read(
        &hi2c1,
        BME280_ADDR,
        0xFA,
        I2C_MEMADD_SIZE_8BIT,
        data,
        3,
        100
        );

    printf("FA=%02X FB=%02X FC=%02X\r\n",
       data[0],
       data[1],
       data[2]);

    adc_T =
     ((uint32_t)data[0] << 12)
   | ((uint32_t)data[1] << 4)
   | ((uint32_t)data[2] >> 4);
    return adc_T;
}