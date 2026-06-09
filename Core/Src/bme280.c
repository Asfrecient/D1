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

#define BME280_REG_DIG_H1    0xA1
#define BME280_REG_DIG_H2    0xE1
#define BME280_REG_DIG_H3    0xE3
#define BME280_REG_DIG_H6    0xE7

#define BME280_REG_HUM_MSB   0xFD

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

// 读取LSB在前、MSB在后的16位数据
// 用于校准参数
static uint16_t BME280_ReadLE16(uint8_t reg)
{
    uint8_t lsb;
    uint8_t msb;
    lsb = BME280_ReadReg(reg);
    msb = BME280_ReadReg(reg + 1);
    return ((uint16_t)msb << 8) | lsb;
}

// 读取LSB在后、MSB在前的16位数据
static uint16_t BME280_ReadBE16(uint8_t reg)
{
    uint8_t msb;
    uint8_t lsb;

    msb = BME280_ReadReg(reg);
    lsb = BME280_ReadReg(reg + 1);

    return ((uint16_t)msb << 8) | lsb;
}



static void BME280_ReadCalibration(void)
{



    bme280_calib.dig_T1 =
        BME280_ReadLE16(BME280_REG_DIG_T1) ;


    bme280_calib.dig_T2 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_T2) ;


    bme280_calib.dig_T3 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_T3) ;

    bme280_calib.dig_H1 =
        BME280_ReadReg(BME280_REG_DIG_H1) ;

    bme280_calib.dig_H2 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_H2) ;

    bme280_calib.dig_H3 =
        BME280_ReadReg(BME280_REG_DIG_H3) ;


    uint8_t e4;
    uint8_t e5;
    uint8_t e6;
    e4 = BME280_ReadReg(0xE4);
    e5 = BME280_ReadReg(0xE5);
    e6 = BME280_ReadReg(0xE6);


    bme280_calib.dig_H4 =
    (int16_t)((e4 << 4) | (e5 & 0x0F));

    bme280_calib.dig_H5 =
        (int16_t)((e6 << 4) | (e5 >> 4));

    bme280_calib.dig_H6 =
    (int8_t)BME280_ReadReg(BME280_REG_DIG_H6);
}

static void BME280_PrintHumidityCalib(void)
{
    printf("H1=%u\r\n", bme280_calib.dig_H1);
    printf("H2=%d\r\n", bme280_calib.dig_H2);
    printf("H3=%u\r\n", bme280_calib.dig_H3);
    printf("H4=%d\r\n", bme280_calib.dig_H4);
    printf("H5=%d\r\n", bme280_calib.dig_H5);
    printf("H6=%d\r\n", bme280_calib.dig_H6);
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

static int32_t BME280_ReadRawHumidity(void)
{
    return BME280_ReadBE16(BME280_REG_HUM_MSB);
}

int32_t BME280_ReadHumidity(void)
{
    int32_t adc_H;
    int32_t v_x1_u32r;


    adc_H = BME280_ReadRawHumidity();

    printf("adc_H=%ld\r\n", adc_H);
    printf("t_fine=%ld\r\n", t_fine);

    v_x1_u32r = t_fine - 76800;
    printf("v1=%ld\r\n", v_x1_u32r);

    int32_t temp1;
    int32_t temp2;
    int32_t temp3;

    temp1 = (adc_H << 14);

    temp2 = ((int32_t)bme280_calib.dig_H4 << 20);

    temp3 = ((int32_t)bme280_calib.dig_H5 * v_x1_u32r);

    printf("temp1=%ld\r\n", temp1);
    printf("temp2=%ld\r\n", temp2);
    printf("temp3=%ld\r\n", temp3);

    v_x1_u32r =
(
    (
        temp1
        - temp2
        - temp3
        + 16384
    )
    >> 15
);

    printf("v2=%ld\r\n", v_x1_u32r);

    v_x1_u32r =
(
    v_x1_u32r *
    (
        (
            (
                (
                    (
                        (v_x1_u32r *
                         ((int32_t)bme280_calib.dig_H6))
                        >> 10
                    )
                    *
                    (
                        (
                            (v_x1_u32r *
                             ((int32_t)bme280_calib.dig_H3))
                            >> 11
                        )
                        + 32768
                    )
                )
                >> 10
            )
            + 2097152
        )
        *
        ((int32_t)bme280_calib.dig_H2)
        + 8192
    )
    >> 14
);

    printf("v3=%ld\r\n", v_x1_u32r);
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
    BME280_PrintHumidityCalib();
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

