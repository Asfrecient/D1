#include "bme280.h"

#include <stdio.h>

#include "i2c.h"

#define BME280_ADDR (0x76 << 1)


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