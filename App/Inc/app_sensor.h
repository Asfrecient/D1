#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include "bme280.h"

#include "app_sensor.h"

extern BME280_Data_t g_latestData;

void APP_SensorRead(BME280_Data_t *data);

#endif