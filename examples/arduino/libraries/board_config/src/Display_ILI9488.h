/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <Arduino.h>
#include "Board_IO.h"

#define LCD_WIDTH BOARD_LCD_WIDTH
#define LCD_HEIGHT BOARD_LCD_HEIGHT
#define LCD_DRAW_BUF_HEIGHT 16

bool LCD_Init();
void LCD_addWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *pixels);
void Set_Backlight(uint8_t percent);
void Backlight_Init();
