#include "app_storage.h"

#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include <string.h>

#define STORAGE_FLASH_ADDR \
0x0800FC00

#define STORAGE_MAGIC \
0x44413130

typedef struct
{
    uint32_t magic;
    AppConfig_t config;
} StorageData_t;

bool Storage_LoadConfig(
    AppConfig_t *cfg)
{
    StorageData_t *data =
        (StorageData_t *)
        STORAGE_FLASH_ADDR;

    if(data->magic !=
       STORAGE_MAGIC)
    {
        return false;
    }

    *cfg = data->config;

    return true;
}

bool Storage_SaveConfig(
    const AppConfig_t *cfg)
{
    StorageData_t data;

    data.magic = STORAGE_MAGIC;
    data.config = *cfg;

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    uint32_t pageError = 0;

    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = STORAGE_FLASH_ADDR;
    erase.NbPages     = 1;

    if(HAL_FLASHEx_Erase(
            &erase,
            &pageError)
       != HAL_OK)
    {
        HAL_FLASH_Lock();
        return false;
    }

    uint32_t *src =
        (uint32_t *)&data;

    uint32_t addr =
        STORAGE_FLASH_ADDR;

    for(uint32_t i = 0;
        i < sizeof(StorageData_t) / 4;
        i++)
    {
        if(HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                addr,
                src[i])
           != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }

        addr += 4;
    }

    HAL_FLASH_Lock();

    return true;
}