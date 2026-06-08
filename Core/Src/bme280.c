#include "bme280.h"

#include <stdio.h>

#include "i2c.h"

#define BME280_ADDR (0x76 << 1)

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

}BME280_Calib_t;


static BME280_Calib_t bme280_calib;
static int32_t t_fine;


//读寄存器
static uint8_t BME280_ReadReg(uint8_t reg)
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
};

static uint16_t BME280_ReadU16(uint8_t reg) {
    uint8_t lsb;
    uint8_t msb;
    lsb = BME280_ReadReg(reg);
    msb = BME280_ReadReg(reg + 1);
    return ((uint16_t)msb << 8) | lsb;
};


static void BME280_ReadCalibration(void)
{
    bme280_calib.dig_T1 =
        BME280_ReadU16(0x88) ;


    bme280_calib.dig_T2 =
        (int16_t)BME280_ReadU16(0x8A) ;


    bme280_calib.dig_T3 =
        (int16_t)BME280_ReadU16(0x8C) ;

}


//写寄存器
static void BME280_WriteReg(uint8_t reg,uint8_t data)
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

static int32_t BME280_ReadRawTemp(void)
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

    adc_T =
     ((uint32_t)data[0] << 12)
   | ((uint32_t)data[1] << 4)
   | ((uint32_t)data[2] >> 4);
    return adc_T;
}

int32_t BME280_ReadTemperature(void)
{
    int32_t var1;
    int32_t var2;
    int32_t adc_T;
    int32_t T;

    adc_T = BME280_ReadRawTemp();


    var1 = ((((adc_T >> 3) - ((int32_t)bme280_calib.dig_T1 << 1))) *
        ((int32_t)bme280_calib.dig_T2)) >> 11;

    var2 =
(
    (
        (
            (
                (adc_T >> 4)
                -
                ((int32_t)bme280_calib.dig_T1)
            )
            *
            (
                (adc_T >> 4)
                -
                ((int32_t)bme280_calib.dig_T1)
            )
        )
        >> 12
    )
    *
    ((int32_t)bme280_calib.dig_T3)
)
>> 14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;

    return T;

}

void BME280_Init(void)
{
    BME280_WriteReg(0xF4,0x27);

    BME280_ReadCalibration();
}