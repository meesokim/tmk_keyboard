/*
Copyright 2014 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "print.h"
#include "util.h"
#include "matrix.h"
#include "debug.h"
#include "action_util.h"
#include "protocol/x1.h"


/*
 * Not use Matrix.
 *
 * ROW: 16(4bits)
 * COL: 16(4bits)
 *
 *    8bit wide
 *   +---------+
 *  0|00 ... 0F|
 *  1|08 ... 1F|
 *  :|   ...   |
 *  :|   ...   |
 *  E|E0 ... EF|
 *  F|F0 ... FF|
 *   +---------+
 */
 
 
#define M_GRAPH  MOD_BIT(KC_LALT)
#define M_CAPS   MOD_BIT(KC_CAPS)
#define M_SHIFT  MOD_BIT(KC_LSHIFT)
#define M_KANA   MOD_BIT(KC_KANA)
#define M_CTRL   MOD_BIT(KC_LCTRL)

#define FUNC  1<<15
#define NOKEY 1<<14
#define REPT  1<<13
#define GRAPH 1<<12
#define CAPS  1<<11
#define SHIFT 1<<10
#define KANA  1<<9
#define CTRL  1<<8

inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

void matrix_init(void)
{
    debug_matrix = true;
    x1_init();

    debug("init\n");
    return;
}

static void type_key(uint16_t keycode)
{
    if (keycode == 0) return;

    uint8_t mods = keycode>>8;
    uint8_t key = keycode&0xFF;
    if (mods) {
        add_mods(mods);
        send_keyboard_report();
    }

    add_key(key);
    send_keyboard_report();

    del_key(key);
    send_keyboard_report();

    if (mods) {
        del_mods(mods);
        send_keyboard_report();
    }
}
static uint16_t code2key(uint16_t wcode)
{
    // ASCII to key combination in US laout
	uint8_t code = (uint8_t) wcode;
	uint16_t mode = ((wcode & GRAPH ? 0 : M_GRAPH) | 
	               (wcode & CAPS  ? 0 : M_CAPS)  | 
				   (wcode & SHIFT ? 0 : M_SHIFT) | 
				   (wcode & KANA  ? 0 : M_KANA)  | 
				   (wcode & CTRL ? 0 : M_CTRL))<<8;
	if (!wcode & NOKEY)
		return mode;
    switch (code) {
        case 0x01 ... 0x08:                                 // Ctrl-[a-z]
            return mode | MOD_BIT(KC_LCTRL)<<8 | (KC_A + (code-0x01));
        case 0x09: return mode | KC_TAB;                           // TAB(Ctrl-i)
        case 0x0A ... 0x0C:                                 // Ctrl-[a-z]
            return mode | MOD_BIT(KC_LCTRL)<<8 | (KC_A + (code-0x01));
        case 0x0D: return mode | KC_ENTER;                         // Enter(Ctrl-m)
        case 0x0E ... 0x1A:                                 // Ctrl-[a-z]
            return mode | MOD_BIT(KC_LCTRL)<<8 | (KC_A + (code-0x01));
        case 0x1B: return mode | KC_ESC;
        case 0x1C: return mode | KC_RIGHT;
        case 0x1D: return mode | KC_LEFT;
        case 0x1E: return mode | KC_UP;
        case 0x1F: return mode | KC_DOWN;
        case 0x20: return mode | KC_SPACE;
        case 0x21: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_1;     // !
        case 0x22: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_QUOTE; // "
        case 0x23: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_3;     // #
        case 0x24: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_4;     // $
        case 0x25: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_5;     // %
        case 0x26: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_7;     // &
        case 0x27: return mode | KC_QUOTE;                         // '
        case 0x28: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_9;     // (
        case 0x29: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_0;     // )
        case 0x2A: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_8;     // *
        case 0x2B: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_EQUAL; // +
        case 0x2C: return mode | KC_COMMA;                         // ,
        case 0x2D: return mode | KC_MINUS;                         // -
        case 0x2E: return mode | KC_DOT;                           // .
        case 0x2F: return mode | KC_SLASH;                         // /
        case 0x30: return mode | KC_0;
        case 0x31 ... 0x39: // 1-9
            return mode | KC_1 + (code-0x31);
        case 0x3A: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_SCLN;  // :
        case 0x3B: return mode | KC_SCLN;                          // ;
        case 0x3C: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_COMMA; // <
        case 0x3D: return mode | KC_EQUAL;                         // =
        case 0x3E: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_DOT;   // >
        case 0x3F: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_SLASH; // ?
        case 0x40: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_2;     // @
        case 0x41 ... 0x5A: // A-Z
            return mode | MOD_BIT(KC_LSHIFT)<<8 | (KC_A + (code-0x41));
        case 0x5B: return mode | KC_LBRACKET;                      // [
        case 0x5C: return mode | KC_BSLASH;                        //
        case 0x5D: return mode | KC_RBRACKET;                      // ]
        case 0x5E: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_6;     // ^
        case 0x5F: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_MINUS; // _
        case 0x61 ... 0x7A: // a-z
            return mode | KC_A + (code-0x61);
        case 0x7B: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_LBRACKET; // {
        case 0x7C: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_BSLASH;   // |
        case 0x7D: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_RBRACKET; // }
        case 0x7E: return mode | MOD_BIT(KC_LSHIFT)<<8 | KC_GRAVE; // }
        case 0x7F: return mode | KC_DELETE;                        //
		case 0xF1: return mode | KC_F1;
		case 0xF2: return mode | KC_F2;
		case 0xF3: return mode | KC_F3;
    }
    return 0;
}

uint8_t matrix_scan(void)
{
    uint16_t code = x1_recv2();
    if (code == -1) {
        return 0;
    }

    print_hex8(code); print(" ");

    // echo back
    //x1_send(code);
    type_key(code2key(code));
    

    return code;
}

inline
bool matrix_has_ghost(void)
{
    return false;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return false;
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return 0;
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < matrix_rows(); row++) {
        phex(row); print(": ");
        pbin_reverse(matrix_get_row(row));
        print("\n");
    }
}
