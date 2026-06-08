#include "bme280.h"

#include <stdio.h>

#include "i2c.h"

#define BME280_ADDR (0x76 << 1)

/* =====================================================
 * 设备信息寄存器
 * ===================================================== */

/* 芯片ID寄存器
 * BME280固定返回0x60
 * 用于确认I2C通信是否正常
 */
#define BME280_REG_ID         0xD0


/* =====================================================
 * 配置寄存器
 * ===================================================== */

/* 湿度过采样配置寄存器
 *
 * 000：跳过湿度测量
 * 001：过采样x1
 * 010：过采样x2
 * ...
 *
 * 注意：
 * 修改后必须再写CTRL_MEAS寄存器才会生效
 */
#define BME280_REG_CTRL_HUM   0xF2


/* 温度/气压测量控制寄存器
 *
 * [7:5] 温度过采样
 * [4:2] 气压过采样
 * [1:0] 工作模式
 *
 * mode:
 * 00 Sleep
 * 01 Forced
 * 10 Forced
 * 11 Normal
 *
 * 例如：
 * 0x27 =
 * 温度x1
 * 气压x1
 * Normal模式
 */
#define BME280_REG_CTRL_MEAS  0xF4


/* =====================================================
 * 温度数据寄存器
 * ===================================================== */

/* 温度原始ADC数据起始寄存器
 *
 * 0xFA -> Temp_MSB
 * 0xFB -> Temp_LSB
 * 0xFC -> Temp_XLSB
 *
 * 三个寄存器共同组成20位ADC值
 *
 * adc_T =
 * (MSB  << 12)
 * (LSB  << 4)
 * (XLSB >> 4)
 */
#define BME280_REG_TEMP_MSB   0xFA


/* =====================================================
* 温度校准参数寄存器
BME280
寄存器
温度补偿参数   DIG
第几个系数
 * ===================================================== */

/* 温度补偿参数T1
 *
 * 类型：uint16_t
 * 地址：
 * 0x88(LSB)
 * 0x89(MSB)
 */
#define BME280_REG_DIG_T1     0x88


/* 温度补偿参数T2
 *
 * 类型：int16_t
 * 地址：
 * 0x8A(LSB)
 * 0x8B(MSB)
 */
#define BME280_REG_DIG_T2     0x8A


/* 温度补偿参数T3
 *
 * 类型：int16_t
 * 地址：
 * 0x8C(LSB)
 * 0x8D(MSB)
 */
#define BME280_REG_DIG_T3     0x8C

typedef struct
{
    /* temperature */
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    /* pressure */
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    /* humidity */
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

} BME280_Calib_t;

#define BME280_REG_DIG_H1    0xA1
#define BME280_REG_DIG_H2    0xE1
#define BME280_REG_DIG_H3    0xE3


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
}

static uint16_t BME280_ReadU16(uint8_t reg)
{
    uint8_t lsb;
    uint8_t msb;
    lsb = BME280_ReadReg(reg);
    msb = BME280_ReadReg(reg + 1);
    return ((uint16_t)msb << 8) | lsb;
}


static void BME280_ReadCalibration(void)
{
    bme280_calib.dig_T1 =
        BME280_ReadU16(BME280_REG_DIG_T1) ;


    bme280_calib.dig_T2 =
        (int16_t)BME280_ReadU16(BME280_REG_DIG_T2) ;


    bme280_calib.dig_T3 =
        (int16_t)BME280_ReadU16(BME280_REG_DIG_T3) ;

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


static int32_t BME280_ReadRawTemp(void)
{

    uint8_t data[3];
    int32_t adc_T;
    HAL_I2C_Mem_Read(
        &hi2c1,
        BME280_ADDR,
        BME280_REG_TEMP_MSB,
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

    BME280_WriteReg(BME280_REG_CTRL_HUM, 0x01);// humidity x1
    BME280_WriteReg(BME280_REG_CTRL_MEAS,0x27);// temp/press x1 + normal

    BME280_ReadCalibration();
}

// static void BME280_DumpCalibration(void)
// {
//     for(uint8_t reg = 0x88;
//         reg <= 0xA1;
//         reg++)
//     {
//         printf("0x%02X = 0x%02X\r\n",
//                 reg,
//                 BME280_ReadReg(reg));
//     }
// }
