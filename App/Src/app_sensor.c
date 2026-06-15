#include "app_sensor.h"

BME280_Data_t g_latestData;

void APP_SensorRead(BME280_Data_t *data)
{
    BME280_ReadData(data);
}