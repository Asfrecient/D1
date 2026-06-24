#ifndef APP_STORAGE_H
#define APP_STORAGE_H

#include <stdbool.h>

#include "app_config.h"


bool Storage_SaveConfig(
    const AppConfig_t *cfg);

bool Storage_LoadConfig(
    AppConfig_t *cfg);

#endif