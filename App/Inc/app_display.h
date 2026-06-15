#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "bme280.h"

void APP_DisplayUpdate(BME280_Data_t *data);

void APP_DisplayShowStack(
    uint32_t sensorStack,
    uint32_t displayStack);

#endif