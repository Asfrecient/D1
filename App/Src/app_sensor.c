#include "app_sensor.h"

void APP_SensorRead(BME280_Data_t *data)
{
    BME280_ReadData(data);
}