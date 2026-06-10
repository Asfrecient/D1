#ifndef __BME280_H
#define __BME280_H

#include "main.h"


typedef struct
{
    int32_t temperature; /* 温度，单位 0.01 摄氏度。 */
    int32_t humidity;    /* 相对湿度，Q22.10 格式，单位 %RH。 */
    int32_t pressure;    /* 气压，Q24.8 格式，单位 Pa。 */
} BME280_Data_t;

/* 初始化 BME280 为正常模式，并读取全部校准参数。 */
void BME280_Init(void);

/* 读取补偿后的温度。
 * 返回值单位为 0.01 摄氏度，例如 2534 表示 25.34 摄氏度。
 */
int32_t BME280_ReadTemperature(void);

/* 读取补偿后的相对湿度。
 * 调用前必须先调用 BME280_ReadTemperature() 更新 t_fine。
 * 返回值为 Q22.10 格式的 %RH，除以 1024 得到实际百分比。
 */
int32_t BME280_ReadHumidity(void);

/* 读取补偿后的气压。
 * 调用前必须先调用 BME280_ReadTemperature() 更新 t_fine。
 * 返回值为 Q24.8 格式的 Pa，除以 256 得到实际 Pa。
 */
int32_t BME280_ReadPressure(void);

/* 一次读取温度、湿度和气压到数据结构。
 * 各字段单位与单独读取函数一致。
 */
void BME280_ReadData(BME280_Data_t *data);

#endif
