#include "gt911_bsp.h"
#include <Wire.h>

namespace {
constexpr uint16_t PRODUCT_ID_REG = 0x8140;
constexpr uint16_t CONFIG_VERSION_REG = 0x8047;
constexpr uint16_t TOUCH_STATUS_REG = 0x814E;
constexpr uint16_t FIRST_POINT_REG = 0x814F;
constexpr uint8_t MAX_TOUCH_POINTS = 5;

uint8_t touch_address = 0;
lv_point_t last_point = {0, 0};
lv_indev_state_t last_state = LV_INDEV_STATE_REL;
lv_indev_t *touch_indev = nullptr;

bool read_register(uint16_t reg, uint8_t *data, size_t len)
{
  Wire.beginTransmission(touch_address);
  Wire.write(static_cast<uint8_t>(reg >> 8));
  Wire.write(static_cast<uint8_t>(reg));
  if (Wire.endTransmission(false) != 0 ||
      Wire.requestFrom(touch_address, static_cast<uint8_t>(len)) != len) {
    return false;
  }
  for (size_t i = 0; i < len; ++i) {
    data[i] = Wire.read();
  }
  return true;
}

bool clear_touch_status()
{
  Wire.beginTransmission(touch_address);
  Wire.write(static_cast<uint8_t>(TOUCH_STATUS_REG >> 8));
  Wire.write(static_cast<uint8_t>(TOUCH_STATUS_REG));
  Wire.write(static_cast<uint8_t>(0));
  return Wire.endTransmission() == 0;
}

void touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  uint8_t status = 0;
  if (!read_register(TOUCH_STATUS_REG, &status, 1)) {
    last_state = LV_INDEV_STATE_REL;
  } else if (status & 0x80) {
    const uint8_t count = status & 0x0F;
    if (count > 0 && count <= MAX_TOUCH_POINTS) {
      uint8_t point[8];
      if (read_register(FIRST_POINT_REG, point, sizeof(point))) {
        // Track the first point, matching the IDF single-pointer input.
        uint16_t x = point[1] | (static_cast<uint16_t>(point[2]) << 8);
        uint16_t y = point[3] | (static_cast<uint16_t>(point[4]) << 8);
        const uint16_t width = lv_disp_get_hor_res(drv->disp);
        const uint16_t height = lv_disp_get_ver_res(drv->disp);
        last_point.x = x < width ? x : width - 1;
        last_point.y = y < height ? y : height - 1;
        last_state = clear_touch_status() ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
      } else {
        last_state = LV_INDEV_STATE_REL;
      }
    } else {
      clear_touch_status();
      last_state = LV_INDEV_STATE_REL;
    }
  }
  // No ready bit means no new sample; retain the last valid press/release.
  data->point = last_point;
  data->state = last_state;
}
} // namespace

bool gt911_init(bool (*set_reset)(bool high))
{
  touch_address = 0;
  last_state = LV_INDEV_STATE_REL;
  if (set_reset == nullptr || !set_reset(false)) {
    return false;
  }
  delay(10);
  if (!set_reset(true)) {
    return false;
  }
  delay(50);

  const uint8_t addresses[] = {0x5D, 0x14};
  for (uint8_t address : addresses) {
    Serial.printf("Probe GT911 at I2C address 0x%02X\n", address);
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      touch_address = address;
      break;
    }
  }
  if (touch_address == 0) {
    Serial.println("GT911 not found at 0x5D or 0x14");
    return false;
  }

  uint8_t id[4] = {};
  uint8_t config_version = 0;
  if (!read_register(PRODUCT_ID_REG, id, sizeof(id)) ||
      !read_register(CONFIG_VERSION_REG, &config_version, 1) ||
      id[0] != '9' || id[1] != '1' || id[2] != '1' ||
      !clear_touch_status()) {
    Serial.println("GT911 identification/init failed");
    touch_address = 0;
    return false;
  }
  Serial.printf("GT911 found at 0x%02X, ID: %.4s, config: %u (polling)\n",
                touch_address, reinterpret_cast<const char *>(id), config_version);
  return true;
}

lv_indev_t *gt911_register_lvgl_input(lv_disp_t *display)
{
  if (touch_address == 0 || display == nullptr) {
    return nullptr;
  }
  if (touch_indev != nullptr) {
    return touch_indev;
  }
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.disp = display;
  indev_drv.read_cb = touch_read;
  touch_indev = lv_indev_drv_register(&indev_drv);
  return touch_indev;
}
