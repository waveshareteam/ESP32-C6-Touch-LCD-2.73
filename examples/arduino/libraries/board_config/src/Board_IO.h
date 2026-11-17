/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"

bool Board_IO_Init(TwoWire &wire = Wire);
bool Board_IO_SetOutput(uint8_t pin, bool level);
bool Board_SetBacklight(uint8_t percent);
bool Board_ReadBatteryVoltage(float &voltage, uint16_t &raw);
bool Board_Touch_SetReset(bool high);
bool Board_LCD_Reset();
