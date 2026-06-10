#include "bme280.h"

#include <stdint.h>

#include "i2c.h"

#include "app_shared.h"

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

#define BME280_REG_DIG_P1    0x8E
#define BME280_REG_DIG_P2    0x90
#define BME280_REG_DIG_P3    0x92
#define BME280_REG_DIG_P4    0x94
#define BME280_REG_DIG_P5    0x96
#define BME280_REG_DIG_P6    0x98
#define BME280_REG_DIG_P7    0x9A
#define BME280_REG_DIG_P8    0x9C
#define BME280_REG_DIG_P9    0x9E
//气压的MSB
#define BME280_REG_PRESS_MSB  0xF7


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


//12位有符号数
// ↓
// 扩展成16位有符号数
static int16_t BME280_SignExtend12(uint16_t value)
{
    if (value & 0x0800)
    {
        value |= 0xF000;
    }

    return (int16_t)value;
}



static void BME280_ReadCalibration(void)
{
    //////////////////温度

    bme280_calib.dig_T1 =
        BME280_ReadLE16(BME280_REG_DIG_T1) ;


    bme280_calib.dig_T2 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_T2) ;


    bme280_calib.dig_T3 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_T3) ;

    ///////////////////////湿度

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
        BME280_SignExtend12(((uint16_t)e4 << 4) | (e5 & 0x0F));

    bme280_calib.dig_H5 =
        BME280_SignExtend12(((uint16_t)e6 << 4) | (e5 >> 4));

    bme280_calib.dig_H6 =
    (int8_t)BME280_ReadReg(BME280_REG_DIG_H6);

    /////////////////////////////气压
    bme280_calib.dig_P1 =
        BME280_ReadLE16(BME280_REG_DIG_P1);

    bme280_calib.dig_P2 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P2);

    bme280_calib.dig_P3 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P3);

    bme280_calib.dig_P4 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P4);

    bme280_calib.dig_P5 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P5);

    bme280_calib.dig_P6 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P6);

    bme280_calib.dig_P7 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P7);

    bme280_calib.dig_P8 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P8);

    bme280_calib.dig_P9 =
        (int16_t)BME280_ReadLE16(BME280_REG_DIG_P9);
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

static int32_t BME280_ReadRawPressure(void)
{
    uint8_t data[3];
    int32_t adc_P;

    HAL_I2C_Mem_Read(
        &hi2c1,
        BME280_ADDR,
        BME280_REG_PRESS_MSB,
        I2C_MEMADD_SIZE_8BIT,
        data,
        3,
        100);

    adc_P =
        ((uint32_t)data[0] << 12)
      | ((uint32_t)data[1] << 4)
      | ((uint32_t)data[2] >> 4);

    return adc_P;
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

int32_t BME280_ReadHumidity(void)
{
    int32_t adc_H;
    int32_t v_x1_u32r;
    int32_t var1;
    int32_t var2;


    adc_H = BME280_ReadRawHumidity();

    v_x1_u32r = t_fine - 76800;

    var1 =
    (
        (
            (adc_H << 14)
            - ((int32_t)bme280_calib.dig_H4 << 20)
            - ((int32_t)bme280_calib.dig_H5 * v_x1_u32r)
            + 16384
        )
        >> 15
    );

    var2 =
        (((((v_x1_u32r * ((int32_t)bme280_calib.dig_H6)) >> 10)
           * (((v_x1_u32r * ((int32_t)bme280_calib.dig_H3)) >> 11)
              + 32768))
          >> 10)
         + 2097152)
        * ((int32_t)bme280_calib.dig_H2)
        + 8192;

    var2 >>= 14;

    v_x1_u32r = var1 * var2;

    v_x1_u32r =
        v_x1_u32r
        - (
            (
                (
                    (v_x1_u32r >> 15)
                    * (v_x1_u32r >> 15)
                )
                >> 7
            )
            * ((int32_t)bme280_calib.dig_H1)
            >> 4
        );

    if (v_x1_u32r < 0)
    {
        v_x1_u32r = 0;
    }
    else if (v_x1_u32r > 419430400)
    {
        v_x1_u32r = 419430400;
    }

    return v_x1_u32r >> 12;
}

int32_t BME280_ReadPressure(void)
{
    int32_t adc_P;

    int64_t var1;
    int64_t var2;
    int64_t p;

    adc_P = BME280_ReadRawPressure();

    var1 = ((int64_t)t_fine) - 128000;

    var2 = var1 * var1 * (int64_t)bme280_calib.dig_P6;

    var2 = var2 +
           ((var1 * (int64_t)bme280_calib.dig_P5) << 17);

    var2 = var2 +
           (((int64_t)bme280_calib.dig_P4) << 35);

    var1 =
    (
        (
            (var1 * var1 * (int64_t)bme280_calib.dig_P3)
            >> 8
        )
        +
        (
            (var1 * (int64_t)bme280_calib.dig_P2)
            << 12
        )
    );

    var1 =
    (
        (
            (((int64_t)1) << 47)
            + var1
        )
        *
        ((int64_t)bme280_calib.dig_P1)
    )
    >> 33;

    if (var1 == 0)
    {
        return 0;
    }

    p = 1048576 - adc_P;

    p = (((p << 31) - var2) * 3125) / var1;

    var1 =
(
    ((int64_t)bme280_calib.dig_P9)
    *
    (p >> 13)
    *
    (p >> 13)
)
>> 25;

    var2 =
    (
        ((int64_t)bme280_calib.dig_P8)
        * p
    )
    >> 19;

    p =
(
    (
        p
        + var1
        + var2
    )
    >> 8
)
+
(
    ((int64_t)bme280_calib.dig_P7)
    << 4
);
return (int32_t)p;
}

void BME280_ReadData(BME280_Data_t *data)
{

    osMutexAcquire(I2CMutex, osWaitForever);
    data->temperature =
        BME280_ReadTemperature();

    data->humidity =
        BME280_ReadHumidity();

    data->pressure =
        BME280_ReadPressure();

    osMutexRelease(I2CMutex);
}


void BME280_Init(void)
{
    BME280_WriteReg(BME280_REG_CTRL_HUM, 0x01);// humidity x1
    BME280_WriteReg(BME280_REG_CTRL_MEAS,0x27);// temp/press x1 + normal
    BME280_ReadCalibration();

}

