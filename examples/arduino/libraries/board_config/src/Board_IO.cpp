/* SPDX-License-Identifier: Apache-2.0 */
#include "Board_IO.h"

static TwoWire *bus = &Wire;
static uint16_t outputs = 0xffff;
static bool initialized;

static bool write_bytes(uint8_t reg, const uint8_t *data, size_t length)
{
    bus->beginTransmission(BOARD_IO_EXTENSION_ADDR);
    bus->write(reg);
    bus->write(data, length);
    return bus->endTransmission() == 0;
}

static bool write_word(uint8_t reg, uint16_t value)
{
    const uint8_t data[] = {uint8_t(value), uint8_t(value >> 8)};
    return write_bytes(reg, data, sizeof(data));
}

bool Board_IO_Init(TwoWire &wire)
{
    if (initialized) return bus == &wire;
    bus = &wire;
    if (!bus->begin(BOARD_I2C_SDA_PIN, BOARD_I2C_SCL_PIN, BOARD_I2C_FREQ_HZ)) return false;
    // CH32V006 uses two bytes for directions and levels; EXIO3 stays input.
    initialized = write_word(BOARD_IO_EXTENSION_OUTPUT_REG, outputs) &&
                  write_word(BOARD_IO_EXTENSION_MODE_REG, 0xfff7);
    return initialized;
}

bool Board_IO_SetOutput(uint8_t pin, bool level)
{
    if (pin > 15 || pin == BOARD_CHARGE_EXIO) return false;
    if (!initialized && !Board_IO_Init(*bus)) return false;
    const uint16_t next = level ? outputs | (1u << pin) : outputs & ~(1u << pin);
    if (!write_word(BOARD_IO_EXTENSION_OUTPUT_REG, next)) return false;
    outputs = next;
    return true;
}

bool Board_SetBacklight(uint8_t percent)
{
    if (!initialized && !Board_IO_Init(*bus)) return false;
    // Preserve the validated 2.73 firmware's PWM limit.
    if (percent > 97) percent = 97;
    const uint8_t value = uint8_t(unsigned(percent) * 255 / 100);
    return write_bytes(BOARD_IO_EXTENSION_PWM_REG, &value, 1);
}

bool Board_ReadBatteryVoltage(float &voltage, uint16_t &raw)
{
    if (!initialized && !Board_IO_Init(*bus)) return false;
    uint32_t sum = 0;
    for (unsigned i = 0; i < 8; ++i) {
        bus->beginTransmission(BOARD_IO_EXTENSION_ADDR);
        bus->write(BOARD_IO_EXTENSION_ADC_REG);
        if (bus->endTransmission(false) != 0 ||
            bus->requestFrom(uint8_t(BOARD_IO_EXTENSION_ADDR), uint8_t(2)) != 2) return false;
        const uint16_t low = bus->read();
        const uint16_t sample = low | (uint16_t(bus->read()) << 8);
        if (sample > 4095) return false;
        sum += sample;
        delay(5);
    }
    const uint16_t average = (sum + 4) / 8;
    raw = average;
    voltage = average * 3.3f / 4095.0f * 3.0f;
    return true;
}

bool Board_Touch_SetReset(bool high)
{
    return Board_IO_SetOutput(BOARD_TP_RST_EXIO, high);
}

bool Board_LCD_Reset()
{
    if (!Board_IO_SetOutput(BOARD_LCD_RST_EXIO, false)) return false;
    delay(10);
    if (!Board_IO_SetOutput(BOARD_LCD_RST_EXIO, true)) return false;
    delay(150);
    return true;
}
