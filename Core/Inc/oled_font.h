#ifndef OLED_FONT_H
#define OLED_FONT_H

#include "main.h"

#define OLED_FONT5X8_WIDTH 5U
#define OLED_FONT5X8_FIRST_CHAR 0x20U
#define OLED_FONT5X8_LAST_CHAR 0x7FU

extern const uint8_t oled_font5x8[][OLED_FONT5X8_WIDTH];

#endif
