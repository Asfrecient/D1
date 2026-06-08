#include "oled.h"

#include <stdio.h>

#include "i2c.h"
#include "oled_font.h"
#include "string.h"

#define OLED_ADDR (0x3CU << 1)
#define OLED_WIDTH 128U
#define OLED_PAGES 8U
#define OLED_CHAR_WIDTH (OLED_FONT5X8_WIDTH + 1U)


static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t data[2];

    data[0] = 0x00;
    data[1] = cmd;

    HAL_I2C_Master_Transmit(
        &hi2c1,
        OLED_ADDR,
        data,
        2,
        100);
}

static void OLED_WriteDataBuffer(const uint8_t *data, uint16_t len)
{
    uint8_t buf[OLED_WIDTH + 1U];

    if(len > OLED_WIDTH)
    {
        len = OLED_WIDTH;
    }

    buf[0] = 0x40;
    memcpy(&buf[1], data, len);

    HAL_I2C_Master_Transmit(
        &hi2c1,
        OLED_ADDR,
        buf,
        len + 1U,
        100);
}

void OLED_Init(void)
{
    printf("OLED Init\r\n");
    HAL_Delay(100);

    OLED_WriteCmd(0xAE);

    OLED_WriteCmd(0x20);
    OLED_WriteCmd(0x10);

    OLED_WriteCmd(0xB0);

    OLED_WriteCmd(0xC8);

    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);

    OLED_WriteCmd(0x40);

    OLED_WriteCmd(0x81);
    OLED_WriteCmd(0xFF);

    OLED_WriteCmd(0xA1);

    OLED_WriteCmd(0xA6);

    OLED_WriteCmd(0xA8);
    OLED_WriteCmd(0x3F);

    OLED_WriteCmd(0xD3);
    OLED_WriteCmd(0x00);

    OLED_WriteCmd(0xD5);
    OLED_WriteCmd(0xF0);

    OLED_WriteCmd(0xD9);
    OLED_WriteCmd(0x22);

    OLED_WriteCmd(0xDA);
    OLED_WriteCmd(0x12);

    OLED_WriteCmd(0xDB);
    OLED_WriteCmd(0x20);

    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x14);

    OLED_WriteCmd(0xAF);
    uint8_t buf[OLED_WIDTH];

    memset(buf, 0x00, sizeof(buf));

    for(uint8_t page = 0; page < OLED_PAGES; page++)
    {
        OLED_WriteCmd(0xB0 + page);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);

        OLED_WriteDataBuffer(buf, sizeof(buf));
    }
}

void OLED_Clear(void)
{
    uint8_t buf[OLED_WIDTH];

    memset(buf, 0x00, sizeof(buf));

    for(uint8_t page = 0; page < OLED_PAGES; page++)
    {
        OLED_WriteCmd(0xB0 + page);

        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);

        OLED_WriteDataBuffer(buf, sizeof(buf));
    }
}

void OLED_Fill(uint8_t data)
{
    uint8_t buf[OLED_WIDTH];

    memset(buf, data, sizeof(buf));

    for(uint8_t page = 0; page < OLED_PAGES; page++)
    {
        OLED_WriteCmd(0xB0 + page);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);

        OLED_WriteDataBuffer(buf, sizeof(buf));
    }
}

void OLED_SetPos(uint8_t x, uint8_t y)
{
    OLED_WriteCmd(0xB0 + y);

    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);

    OLED_WriteCmd(x & 0x0F);
}

void OLED_ShowChar(uint8_t x, uint8_t page, char ch)
{
    uint8_t buf[OLED_CHAR_WIDTH];
    uint8_t index;

    if(x >= OLED_WIDTH || page >= OLED_PAGES)
    {
        return;
    }

    if((uint8_t)ch < OLED_FONT5X8_FIRST_CHAR || (uint8_t)ch > OLED_FONT5X8_LAST_CHAR)
    {
        ch = '?';
    }

    index = (uint8_t)ch - OLED_FONT5X8_FIRST_CHAR;
    memcpy(buf, oled_font5x8[index], OLED_FONT5X8_WIDTH);
    buf[OLED_FONT5X8_WIDTH] = 0x00;

    OLED_SetPos(x, page);
    OLED_WriteDataBuffer(buf, OLED_CHAR_WIDTH);
}

void OLED_ShowString(uint8_t x, uint8_t page, const char *str)
{
    while(str != NULL && *str != '\0' && page < OLED_PAGES)
    {
        if(*str == '\n')
        {
            x = 0U;
            page++;
            str++;
            continue;
        }

        if(x + OLED_CHAR_WIDTH > OLED_WIDTH)
        {
            x = 0U;
            page++;
            if(page >= OLED_PAGES)
            {
                break;
            }
        }

        OLED_ShowChar(x, page, *str++);
        x += OLED_CHAR_WIDTH;
    }
}
