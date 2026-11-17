/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <Arduino.h>
struct RtcDateTime_t {
    uint16_t year;
    uint8_t month, day, hour, minute, second, week;
};
bool rtc_init();
bool i2c_rtc_setTime(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);
bool i2c_rtc_get(RtcDateTime_t &time);
