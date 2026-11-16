/* SPDX-License-Identifier: Apache-2.0 */
#include "rtc_bsp.h"
#include <Wire.h>
static constexpr uint8_t address = 0x51;
static bool read_register(uint8_t reg, uint8_t *data, uint8_t length)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0 || Wire.requestFrom(address, length) != length) return false;
    for (uint8_t i = 0; i < length; ++i) data[i] = Wire.read();
    return true;
}
static bool write_register(uint8_t reg, const uint8_t *data, uint8_t length)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data, length);
    return Wire.endTransmission() == 0;
}
bool rtc_init()
{
    // Preserve other control bits; clear STOP and select 24-hour operation.
    uint8_t control;
    if (!read_register(0, &control, 1)) return false;
    control &= ~0x22;
    return write_register(0, &control, 1);
}
static uint8_t bcd(uint8_t value) { return ((value / 10) << 4) | (value % 10); }
bool i2c_rtc_setTime(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second)
{
    // Calculate the weekday for a Gregorian calendar date (Sunday = 0).
    const unsigned offsets[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour > 23 || minute > 59 || second > 59) return false;
    unsigned y = year - (month < 3 ? 1 : 0);
    const uint8_t week = (y + y/4 - y/100 + y/400 + offsets[month-1] + day) % 7;
    const uint8_t data[] = {bcd(second),bcd(minute),bcd(hour),bcd(day),week,bcd(month),bcd(year%100)};
    return write_register(4, data, sizeof(data));
}
static bool decimal(uint8_t value, uint8_t &out)
{
    if ((value & 15) > 9 || (value >> 4) > 9) return false;
    out = (value >> 4) * 10 + (value & 15);
    return true;
}
bool i2c_rtc_get(RtcDateTime_t &time)
{
    uint8_t bytes[7], year;
    RtcDateTime_t result = {};
    if (!read_register(4, bytes, sizeof(bytes)) || (bytes[0] & 0x80) ||
        !decimal(bytes[0] & 0x7F,result.second) || !decimal(bytes[1] & 0x7F,result.minute) ||
        !decimal(bytes[2] & 0x3F,result.hour) || !decimal(bytes[3] & 0x3F,result.day) ||
        !decimal(bytes[5] & 0x1F,result.month) || !decimal(bytes[6],year)) return false;
    result.year = 2000 + year;
    result.week = bytes[4] & 7;
    if (result.second > 59 || result.minute > 59 || result.hour > 23 || result.month < 1 ||
        result.month > 12 || result.day < 1 || result.week > 6) return false;
    const uint8_t days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    unsigned maximum = days[result.month-1] + (result.month == 2 && result.year % 4 == 0 ? 1 : 0);
    if (result.day > maximum) return false;
    time = result;
    return true;
}
