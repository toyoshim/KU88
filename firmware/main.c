// Copyright 2022 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
#include <stdint.h>

#include "ch559.h"
#include "hid.h"
#include "key.h"
#include "led.h"
#include "serial.h"

static bool button_check(uint16_t index, const uint8_t* data) {
  if (index == 0xffff)
    return false;
  uint8_t byte = index >> 3;
  uint8_t bit = index & 7;
  return data[byte] & (1 << bit);
}

static int8_t axis_check(const struct hub_info* info,
                         const uint8_t* data,
                         uint8_t index) {
  if (info->axis[index] == 0xffff)
    return 0;
  if (info->axis_size[index] == 8) {
    uint8_t v = data[info->axis[index] >> 3];
    if (info->axis_sign[index])
      v += 0x80;
    if (info->axis_polarity[index])
      v = 0xff - v;
    if (v < 0x60)
      return -1;
    if (v > 0xa0)
      return 1;
  } else if (info->axis_size[index] == 12) {
    uint8_t byte_index = info->axis[index] >> 3;
    uint16_t l = data[byte_index + 0];
    uint16_t h = data[byte_index + 1];
    uint16_t v = ((info->axis[index] & 7) == 0) ? (((h << 8) & 0x0f00) | l)
                                                : ((h << 4) | (l >> 4));
    if (info->axis_sign[index])
      v += 0x0800;
    if (info->axis_polarity[index])
      v = 0x0fff - v;
    if (v < 0x0600)
      return -1;
    if (v > 0x0a00)
      return 1;
  } else if (info->axis_size[index] == 16) {
    uint8_t byte = info->axis[index] >> 3;
    uint16_t v = data[byte] | ((uint16_t)data[byte + 1] << 8);
    if (info->axis_sign[index])
      v += 0x8000;
    if (info->axis_polarity[index])
      v = 0xffff - v;
    if (v < 0x6000)
      return -1;
    if (v > 0xa000)
      return 1;
  }
  return 0;
}

static void report(uint8_t hub,
                   const struct hub_info* info,
                   const uint8_t* data,
                   uint16_t size) {
  hub;
  size;

  if (info->type == HID_TYPE_KEYBOARD)
    return;
  if (info->report_id) {
    if (info->report_id != data[0])
      return;
    data++;
  }

  uint8_t u = button_check(info->dpad[0], data) ? 1 : 0;
  uint8_t d = button_check(info->dpad[1], data) ? 1 : 0;
  uint8_t l = button_check(info->dpad[2], data) ? 1 : 0;
  uint8_t r = button_check(info->dpad[3], data) ? 1 : 0;
  int8_t x = axis_check(info, data, 0);
  int8_t y = axis_check(info, data, 1);
  if (x < 0)
    l = 1;
  else if (x > 0)
    r = 1;
  if (y < 0)
    u = 1;
  else if (y > 0)
    d = 1;
  if (info->hat != 0xffff) {
    uint8_t byte = info->hat >> 3;
    uint8_t bit = info->hat & 7;
    uint8_t hat = (data[byte] >> bit) & 0xf;
    switch (hat) {
      case 0:
        u = 1;
        break;
      case 1:
        u = 1;
        r = 1;
        break;
      case 2:
        r = 1;
        break;
      case 3:
        r = 1;
        d = 1;
        break;
      case 4:
        d = 1;
        break;
      case 5:
        d = 1;
        l = 1;
        break;
      case 6:
        l = 1;
        break;
      case 7:
        l = 1;
        u = 1;
        break;
    }
  }

  // MD mini pad
  //                SELECT
  //  +   START   4 1 L1
  //              3 2 R1
  key_flip(0x0, 7, u && l);                                           // 7
  key_flip(0x1, 0, u);                                                // 8
  key_flip(0x1, 1, u && r);                                           // 9
  key_flip(0x0, 1, d && l);                                           // 1
  key_flip(0x0, 2, d);                                                // 2
  key_flip(0x0, 3, d && r);                                           // 3
  key_flip(0x0, 4, l);                                                // 4
  key_flip(0x0, 6, r);                                                // 6
  key_flip(0x1, 7, button_check(info->button[HID_BUTTON_3], data));   // RET
  key_flip(0x9, 6, button_check(info->button[HID_BUTTON_2], data));   // SPACE
  key_flip(0x9, 7, button_check(info->button[HID_BUTTON_R1], data));  // ESC
  key_flip(0x9, 1, button_check(info->button[HID_BUTTON_4], data));   // F1
  key_flip(0x9, 2, button_check(info->button[HID_BUTTON_1], data));   // F2
  key_flip(0x9, 3, button_check(info->button[HID_BUTTON_L1], data));  // F3
}

static void detected() {
  led_oneshot(L_PULSE_ONCE);
}

static uint8_t get_flags() {
  return USE_HUB0;
}

void main() {
  initialize();

  led_init(1, 4, LOW);
  led_mode(L_BLINK);

  key_init();

  struct hid hid;
  hid.report = report;
  hid.detected = detected;
  hid.get_flags = get_flags;
  hid_init(&hid);

  Serial.println("hello");

  for (;;) {
    led_poll();
    hid_poll();
    key_flush();
  }
}