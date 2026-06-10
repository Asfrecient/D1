#include "app_display.h"

#include <stdio.h>
#include <stdlib.h>

#include "bme280.h"
#include "oled.h"

#include "app_shared.h"

extern BME280_Data_t sensor;

void APP_DisplayUpdate(void)
{
    char str[40];

    sprintf(str,
            "T:%ld.%02ldC",
            sensor.temperature / 100,
            labs(sensor.temperature % 100));

    OLED_ShowString(0,0,str);

    sprintf(str,
            "H:%2ld.%02ld%%",
            sensor.humidity / 1024,
            (sensor.humidity % 1024) * 100 / 1024);

    OLED_ShowString(0,2,str);

    sprintf(str,
            "P:%ld.%02ldhPa",
            sensor.pressure / 256 / 100,
            sensor.pressure / 256 % 100);

    OLED_ShowString(0,4,str);
}