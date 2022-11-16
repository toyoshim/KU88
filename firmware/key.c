// Copyright 2022 Takashi Toyoshima <toyoshim@gmail.com>.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
#include "key.h"

#include "ch559.h"
#include "io.h"
#include "uart1.h"

static bool dirty_rows[0x0f] = {false, false, false, false, false,
                                false, false, false, false, false,
                                false, false, false, false, false};

static uint8_t rows[0x0f] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
};

static volatile uint8_t int_count = 0;
static volatile uint16_t int_data = 0;

void key_int_tmr0() __interrupt INT_NO_TMR0 __using 0 {
  if (0 == int_count)
    return;
  if (int_count < 15) {
    // 1: Start-bit
    // 2-5: R0-3
    // 6-13: D0-D7
    // 14: Parity
    P2_7 = int_data & 1;
    int_data >>= 1;
  } else {
    P2_7 = 1;
  }
  // reset to 0 after 50t blank period.
  int_count = (int_count + 1) & 0x3f;
}

static void update_row(uint8_t row, uint8_t data) {
  uint16_t bits = (row & 0x0f) | (data << 4);
  uint8_t popc = 0;
  for (uint16_t bit = 1; bit != (1 << 12); bit <<= 1) {
    if (bits & bit)
      popc++;
  }
  if (popc & 1)
    bits |= (1 << 12);
  while (int_count)
    ;
  int_data = bits << 1;
  int_count = 1;
}

void key_init() {
  uart1_init(UART1_P4, UART1_115200);  // Release P2_7.
  pinMode(2, 7, OUTPUT);
  digitalWrite(2, 7, HIGH);

  TMOD = (TMOD | bT0_M1) & ~bT0_M0;  // Timer0 mode 2
  TL0 = 0;
  TH0 = 64;  // Fsys (48MHz) / 12 / (256 - TH) = 20800 (1T = 48usec)
  ET0 = 1;   // Timer0 interrupt enable.
  EA = 1;    // Global interrupt enable.
  TR0 = 1;   // Start timer count.
}

void key_reset() {
  for (int i = 0; i < 14; ++i)
    rows[i] = 0xff;
  rows[14] = 0x7f;
}

void key_flip(uint8_t row, uint8_t data, bool set) {
  uint8_t mask = 1 << data;
  bool was_set = 0 == (rows[row] & mask);
  if (was_set != set) {
    dirty_rows[row] = true;
    if (set) {
      rows[row] &= ~mask;
    } else {
      rows[row] |= mask;
    }
  }

  // TODO: emmit legacy combination for new keys.
}

void key_flush() {
  for (uint8_t i = 0; i < 0x0f; ++i) {
    if (!dirty_rows[i])
      continue;
    dirty_rows[i] = false;
    update_row(i, rows[i]);
  }
}