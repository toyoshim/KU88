// Copyright 2022 Takashi Toyoshima <toyoshim@gmail.com>.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __key_h__
#define __key_h__

#include <stdbool.h>
#include <stdint.h>

#include "interrupt.h"

extern void key_int_tmr0() __interrupt INT_NO_TMR0 __using 0;

// Row \ Data | 0    | 1      | 2      | 3      | 4    | 5   | 6     | 7
//------------+------+--------+--------+--------+------+-----+-------+---
// (10-Key) 0 | 0    | 1      | 2      | 3      | 4    | 5   | 6     | 7
// (10-Key) 1 | 8    | 9      | *      | +      | =    | ,   | .     | RET
//          2 | @    | A      | B      | C      | D    | E   | F     | G
//          3 | H    | I      | J      | K      | L    | M   | N     | O
//          4 | P    | Q      | R      | S      | T    | U   | V     | W
//          5 | X    | Y      | Z      | [      | \    | ]   | ^     | -
//          6 | 0    | 1      | 2      | 3      | 4    | 5   | 6     | 7
//          7 | 8    | 9      | :      | ;      | ,    | .   | /     | _
//          8 | HOME | ↑      | →      | INSDEL | GRPH | カナ | SHIFT | CTRL
//          9 | STOP | F1     | F2     | F3     | F4   | F5  | SPACE | ESC
//          A | TAB  | ↓      | ←      | HELP   | COPY | -   | /     | CAPS
//          B | R UP | R DOWN |        |        |      |     |       |
//          C | F6   | F7     | F8     | F9     | F10  | BS  | INS   | DEL
//          D | 変換  | 決定   | PC     | 全角    |      |     |       |
//          E | RET  | NUMRET | LSHIFT | RSHIFT |      |     |       | ZERO
void key_init();
void key_reset();
void key_flip(uint8_t row, uint8_t data, bool set);
void key_flush();

#endif  // __key_h__