#include <Demo_UI.h>
#include <pin_config.h>
#include <Arduino.h>
#include <Wire.h>

// Shared 2.73-inch board I2C pins.
static constexpr uint8_t I2C_SDA = BOARD_I2C_SDA_PIN;
static constexpr uint8_t I2C_SCL = BOARD_I2C_SCL_PIN;
static constexpr uint32_t I2C_FREQ = 400000;

static constexpr uint8_t SHTC3_ADDR = 0x70;
static constexpr uint16_t SHTC3_CMD_WAKE = 0x3517;
static constexpr uint16_t SHTC3_CMD_SLEEP = 0xB098;
static constexpr uint16_t SHTC3_CMD_SOFT_RESET = 0x805D;
static constexpr uint16_t SHTC3_CMD_READ_ID = 0xEFC8;
static constexpr uint16_t SHTC3_CMD_MEASURE_T_RH = 0x7866; // T first, no clock stretching, normal mode

static uint8_t shtc3Crc(const uint8_t *data, size_t len)
{
  uint8_t crc = 0xFF;

  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
  }

  return crc;
}

static bool writeCommand(uint16_t cmd)
{
  Wire.beginTransmission(SHTC3_ADDR);
  Wire.write(cmd >> 8);
  Wire.write(cmd & 0xFF);
  return Wire.endTransmission() == 0;
}

static bool wakeShtc3()
{
  if (!writeCommand(SHTC3_CMD_WAKE)) {
    return false;
  }

  delayMicroseconds(240);
  return true;
}

static void sleepShtc3()
{
  writeCommand(SHTC3_CMD_SLEEP);
}

static bool readWordWithCrc(uint16_t *value)
{
  uint8_t buf[3] = {0};

  if (Wire.requestFrom(SHTC3_ADDR, static_cast<uint8_t>(3)) != 3) {
    return false;
  }

  for (uint8_t i = 0; i < sizeof(buf); i++) {
    buf[i] = Wire.read();
  }

  if (shtc3Crc(buf, 2) != buf[2]) {
    return false;
  }

  *value = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
  return true;
}

static bool readShtc3Id(uint16_t *id)
{
  if (!wakeShtc3()) {
    return false;
  }

  bool ok = writeCommand(SHTC3_CMD_READ_ID) && readWordWithCrc(id);
  sleepShtc3();
  return ok;
}

static bool readShtc3(float *temperature, float *humidity)
{
  uint8_t buf[6] = {0};

  if (!wakeShtc3()) {
    return false;
  }

  if (!writeCommand(SHTC3_CMD_MEASURE_T_RH)) {
    sleepShtc3();
    return false;
  }

  delay(20);

  if (Wire.requestFrom(SHTC3_ADDR, static_cast<uint8_t>(6)) != 6) {
    sleepShtc3();
    return false;
  }

  for (uint8_t i = 0; i < sizeof(buf); i++) {
    buf[i] = Wire.read();
  }

  sleepShtc3();

  if (shtc3Crc(&buf[0], 2) != buf[2] || shtc3Crc(&buf[3], 2) != buf[5]) {
    return false;
  }

  uint16_t rawTemp = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
  uint16_t rawHumidity = (static_cast<uint16_t>(buf[3]) << 8) | buf[4];

  *temperature = -45.0f + 175.0f * rawTemp / 65535.0f;
  *humidity = 100.0f * rawHumidity / 65535.0f;
  return true;
}

static lv_obj_t *temperature_label, *humidity_label;
static bool screen_ready;
static uint32_t last_sample;
void setup()
{
    Serial.begin(115200);
    delay(200);
    screen_ready = demo_start("ENVIRONMENT", "SHTC3 / temperature & humidity");
    if (!screen_ready) return;
    temperature_label = demo_metric("TEMPERATURE", 14, 92, 140, 140, &lv_font_montserrat_28);
    humidity_label = demo_metric("HUMIDITY", 166, 92, 140, 140, &lv_font_montserrat_28);
    demo_text(lv_obj_get_parent(temperature_label), "degrees C", 10, 99, 120, LV_FONT_DEFAULT, 0x63758c);
    demo_text(lv_obj_get_parent(humidity_label), "% RH", 10, 99, 120, LV_FONT_DEFAULT, 0x63758c);
    demo_text(demo_screen(), "Live measurement / refreshed each second", 14, 248, 292, LV_FONT_DEFAULT, 0x63758c);
    if (wakeShtc3()) { writeCommand(SHTC3_CMD_SOFT_RESET); delay(2); }
    last_sample = millis() - 1000;
}
void loop()
{
    demo_tick();
    if (screen_ready && millis() - last_sample >= 1000) {
        last_sample = millis();
        float temperature = 0, humidity = 0;
        if (readShtc3(&temperature, &humidity)) {
            char text[24];
            snprintf(text, sizeof(text), "%.1f", temperature);
            lv_label_set_text(temperature_label, text);
            snprintf(text, sizeof(text), "%.1f", humidity);
            lv_label_set_text(humidity_label, text);
            demo_status_set("Sensor online / CRC checked", false);
            Serial.printf("%.2f C; %.2f %%RH\n",temperature,humidity);
        } else {
            lv_label_set_text(temperature_label, "--");
            lv_label_set_text(humidity_label, "--");
            demo_status_set("SHTC3 read/CRC error / retrying", true);
        }
    }
    delay(5);
}
