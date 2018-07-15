#ifndef _KBD_SETS_H_
#define _KBD_SETS_H_

#include "keyboard.h"

struct {
	char c;
	kbd_code_type tp;
} kbd_set[3][0x100] = {
#define key(i, chr, type) [i] = {.c = chr, .tp = type},
	[KBD_SET_1] = {
		key(0x00, '\0', KBD_INVALID) // None
		key(0x01, '?', KBD_PRESSED_SPECIAL) // Escape
		key(0x02, '1', KBD_PRESSED)
		key(0x03, '2', KBD_PRESSED)
		key(0x04, '3', KBD_PRESSED)
		key(0x05, '4', KBD_PRESSED)
		key(0x06, '5', KBD_PRESSED)
		key(0x07, '6', KBD_PRESSED)
		key(0x08, '7', KBD_PRESSED)
		key(0x09, '8', KBD_PRESSED)
		key(0x0A, '9', KBD_PRESSED)
		key(0x0B, '0', KBD_PRESSED)
		key(0x0C, '-', KBD_PRESSED)
		key(0x0D, '=', KBD_PRESSED)
		key(0x0E, '\b', KBD_PRESSED_SPECIAL) // Backspace
		key(0x0F, '\t', KBD_PRESSED)
		key(0x10, 'q', KBD_PRESSED)
		key(0x11, 'w', KBD_PRESSED)
		key(0x12, 'e', KBD_PRESSED)
		key(0x13, 'r', KBD_PRESSED)
		key(0x14, 't', KBD_PRESSED)
		key(0x15, 'y', KBD_PRESSED)
		key(0x16, 'u', KBD_PRESSED)
		key(0x17, 'i', KBD_PRESSED)
		key(0x18, 'o', KBD_PRESSED)
		key(0x19, 'p', KBD_PRESSED)
		key(0x1A, '[', KBD_PRESSED)
		key(0x1B, ']', KBD_PRESSED)
		key(0x1C, '\n', KBD_PRESSED_SPECIAL) // Enter
		key(0x1D, 'c', KBD_PRESSED_SPECIAL) // Left Control
		key(0x1E, 'a', KBD_PRESSED)
		key(0x1F, 's', KBD_PRESSED)
		key(0x20, 'd', KBD_PRESSED)
		key(0x21, 'f', KBD_PRESSED)
		key(0x22, 'g', KBD_PRESSED)
		key(0x23, 'h', KBD_PRESSED)
		key(0x24, 'j', KBD_PRESSED)
		key(0x25, 'k', KBD_PRESSED)
		key(0x26, 'l', KBD_PRESSED)
		key(0x27, ';', KBD_PRESSED)
		key(0x28, '\'', KBD_PRESSED)
		key(0x29, '`', KBD_PRESSED)
		key(0x2A, 's', KBD_PRESSED_SPECIAL) // Left Shift
		key(0x2B, '\\', KBD_PRESSED)
		key(0x2C, 'z', KBD_PRESSED)
		key(0x2D, 'x', KBD_PRESSED)
		key(0x2E, 'c', KBD_PRESSED)
		key(0x2F, 'v', KBD_PRESSED)
		key(0x30, 'b', KBD_PRESSED)
		key(0x31, 'n', KBD_PRESSED)
		key(0x32, 'm', KBD_PRESSED)
		key(0x33, ',', KBD_PRESSED)
		key(0x34, '.', KBD_PRESSED)
		key(0x35, '/', KBD_PRESSED)
		key(0x36, 's', KBD_PRESSED_SPECIAL) // Right Shift
		key(0x37, '*', KBD_PRESSED)
		key(0x38, 'a', KBD_PRESSED_SPECIAL) // Left Alt
		key(0x39, ' ', KBD_PRESSED_SPECIAL) // Space
		key(0x3A, 'C', KBD_PRESSED_SPECIAL) // Caps
		key(0x3B, '1', KBD_PRESSED_SPECIAL) // F1
		key(0x3C, '2', KBD_PRESSED_SPECIAL) // F2
		key(0x3D, '3', KBD_PRESSED_SPECIAL) // F3
		key(0x3E, '4', KBD_PRESSED_SPECIAL) // F4
		key(0x3F, '5', KBD_PRESSED_SPECIAL) // F5
		key(0x40, '6', KBD_PRESSED_SPECIAL) // F6
		key(0x41, '7', KBD_PRESSED_SPECIAL) // F7
		key(0x42, '8', KBD_PRESSED_SPECIAL) // F8
		key(0x43, '9', KBD_PRESSED_SPECIAL) // F9
		key(0x44, '0', KBD_PRESSED_SPECIAL) // F10
		key(0x45, 'N', KBD_PRESSED_SPECIAL) // NUMLOCK
		key(0x46, 'S', KBD_PRESSED_SPECIAL) // SCROLLLOCK
		// Keypad
		key(0x47, '7', KBD_PRESSED)
		key(0x48, '8', KBD_PRESSED)
		key(0x49, '9', KBD_PRESSED)
		key(0x4A, '-', KBD_PRESSED_SPECIAL)
		key(0x4B, '4', KBD_PRESSED)
		key(0x4C, '5', KBD_PRESSED)
		key(0x4D, '6', KBD_PRESSED)
		key(0x4E, '+', KBD_PRESSED_SPECIAL)
		key(0x4F, '1', KBD_PRESSED)
		key(0x50, '2', KBD_PRESSED)
		key(0x51, '3', KBD_PRESSED)
		key(0x52, '0', KBD_PRESSED)
		key(0x53, '.', KBD_PRESSED_SPECIAL) // DEL
		// Should be invalid
		// ...
		key(0x57, '1', KBD_PRESSED_SPECIAL) // F11
		key(0x58, '2', KBD_PRESSED_SPECIAL) // F12
		key(0x81, 'e', KBD_RELEASED_SPECIAL) // Escape
		key(0x82, '1', KBD_RELEASED)
		key(0x83, '2', KBD_RELEASED)
		key(0x84, '3', KBD_RELEASED)
		key(0x85, '4', KBD_RELEASED)
		key(0x86, '5', KBD_RELEASED)
		key(0x87, '6', KBD_RELEASED)
		key(0x88, '7', KBD_RELEASED)
		key(0x89, '8', KBD_RELEASED)
		key(0x8A, '9', KBD_RELEASED)
		key(0x8B, '0', KBD_RELEASED)
		key(0x8C, '-', KBD_RELEASED)
		key(0x8D, '=', KBD_RELEASED)
		key(0x8E, '=', KBD_RELEASED_SPECIAL) // Backspace
		key(0x8F, '\t', KBD_RELEASED)
		key(0x90, 'q', KBD_RELEASED)
		key(0x91, 'w', KBD_RELEASED)
		key(0x92, 'e', KBD_RELEASED)
		key(0x93, 'r', KBD_RELEASED)
		key(0x94, 't', KBD_RELEASED)
		key(0x95, 'y', KBD_RELEASED)
		key(0x96, 'u', KBD_RELEASED)
		key(0x97, 'i', KBD_RELEASED)
		key(0x98, 'o', KBD_RELEASED)
		key(0x99, 'p', KBD_RELEASED)
		key(0x9A, '[', KBD_RELEASED)
		key(0x9B, ']', KBD_RELEASED)
		key(0x9C, '\n', KBD_RELEASED_SPECIAL) // Enter
		key(0x9D, 'c', KBD_RELEASED_SPECIAL) // Left Control
		key(0x9E, 'a', KBD_RELEASED)
		key(0x9F, 's', KBD_RELEASED)
		key(0xA0, 'd', KBD_RELEASED)
		key(0xA1, 'f', KBD_RELEASED)
		key(0xA2, 'g', KBD_RELEASED)
		key(0xA3, 'h', KBD_RELEASED)
		key(0xA4, 'j', KBD_RELEASED)
		key(0xA5, 'k', KBD_RELEASED)
		key(0xA6, 'l', KBD_RELEASED)
		key(0xA7, ';', KBD_RELEASED)
		key(0xA8, '\'', KBD_RELEASED)
		key(0xA9, '`', KBD_RELEASED)
		key(0xAA, 's', KBD_RELEASED_SPECIAL) // Left Shift
		key(0xAB, '\\', KBD_RELEASED)
		key(0xAC, 'z', KBD_RELEASED)
		key(0xAD, 'x', KBD_RELEASED)
		key(0xAE, 'c', KBD_RELEASED)
		key(0xAF, 'v', KBD_RELEASED)
		key(0xB0, 'b', KBD_RELEASED)
		key(0xB1, 'n', KBD_RELEASED)
		key(0xB2, 'm', KBD_RELEASED)
		key(0xB3, ',', KBD_RELEASED)
		key(0xB4, '.', KBD_RELEASED)
		key(0xB5, '/', KBD_RELEASED)
		key(0xB6, 's', KBD_RELEASED_SPECIAL) // Right Shift
		key(0xB7, '*', KBD_RELEASED)
		key(0xB8, 'a', KBD_RELEASED_SPECIAL) // Left Alt
		key(0xB9, ' ', KBD_RELEASED_SPECIAL) // Space
		key(0xBA, 'C', KBD_RELEASED_SPECIAL) // Caps
		key(0xBB, '1', KBD_RELEASED_SPECIAL) // F1
		key(0xBC, '2', KBD_RELEASED_SPECIAL) // F2
		key(0xBD, '3', KBD_RELEASED_SPECIAL) // F3
		key(0xBE, '4', KBD_RELEASED_SPECIAL) // F4
		key(0xBF, '5', KBD_RELEASED_SPECIAL) // F5
		key(0xC0, '6', KBD_RELEASED_SPECIAL) // F6
		key(0xC1, '7', KBD_RELEASED_SPECIAL) // F7
		key(0xC2, '8', KBD_RELEASED_SPECIAL) // F8
		key(0xC3, '9', KBD_RELEASED_SPECIAL) // F9
		key(0xC4, '0', KBD_RELEASED_SPECIAL) // F10
		key(0xC5, 'N', KBD_RELEASED_SPECIAL) // NUMLOCK
		key(0xC6, 'S', KBD_RELEASED_SPECIAL) // SCROLLLOCK
		// Keypad
		key(0xC7, '7', KBD_RELEASED)
		key(0xC8, '8', KBD_RELEASED)
		key(0xC9, '9', KBD_RELEASED)
		key(0xCA, '-', KBD_RELEASED_SPECIAL)
		key(0xCB, '4', KBD_RELEASED)
		key(0xCC, '5', KBD_RELEASED)
		key(0xCD, '6', KBD_RELEASED)
		key(0xCE, '+', KBD_RELEASED)
		key(0xCF, '1', KBD_RELEASED)
		key(0xD0, '2', KBD_RELEASED)
		key(0xD1, '3', KBD_RELEASED)
		key(0xD2, '0', KBD_RELEASED)
		key(0xD3, '.', KBD_RELEASED_SPECIAL) // DEL
		// Should be invalid
		// ...
		key(0xD7, '1', KBD_RELEASED_SPECIAL) // F11
		key(0xD8, '2', KBD_RELEASED_SPECIAL) // F12
	}
};
#undef key


#endif /* _KBD_SETS_H_ */
