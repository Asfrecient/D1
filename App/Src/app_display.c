#include "app_display.h"

#include <stdio.h>
#include <stdlib.h>

#include "bme280.h"
#include "oled.h"

#include "app_shared.h"

void APP_DisplayUpdate(BME280_Data_t *data)
{
        char str[40];

        sprintf(str,
                "T:%ld.%02ldC",
                data->temperature / 100,
                labs(data->temperature % 100));

        OLED_ShowString(0, 0, str);

        sprintf(str,
                "H:%2ld.%02ld%%",
                data->humidity / 1024,
                (data->humidity % 1024) * 100 / 1024);

        OLED_ShowString(0, 2, str);

        sprintf(str,
                "P:%ld.%02ldhPa",
                data->pressure / 256 / 100,
                (data->pressure / 256) % 100);

        OLED_ShowString(0, 4, str);
}