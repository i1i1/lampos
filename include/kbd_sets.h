#ifndef _KBD_SETS_H_
#define _KBD_SETS_H_

#include "stdint.h"
#include "keyboard.h"

#define KBD_BUF_MAX	8

enum kbd_key_enum {
	KBD_KEY_ESCAPE,
	KBD_KEY_0,
	KBD_KEY_1,
	KBD_KEY_2,
	KBD_KEY_3,
	KBD_KEY_4,
	KBD_KEY_5,
	KBD_KEY_6,
	KBD_KEY_7,
	KBD_KEY_8,
	KBD_KEY_9,
	KBD_KEY_MINUS,
	KBD_KEY_EQUAL,
	KBD_KEY_BACKSPACE,
	KBD_KEY_TAB,
	KBD_KEY_Q,
	KBD_KEY_W,
	KBD_KEY_E,
	KBD_KEY_R,
	KBD_KEY_T,
	KBD_KEY_Y,
	KBD_KEY_U,
	KBD_KEY_I,
	KBD_KEY_O,
	KBD_KEY_P,
	KBD_KEY_OPEN_BRACKET,
	KBD_KEY_CLOSE_BRACKET,
	KBD_KEY_ENTER,
	KBD_KEY_LEFT_CONTROL,
	KBD_KEY_A,
	KBD_KEY_S,
	KBD_KEY_D,
	KBD_KEY_F,
	KBD_KEY_G,
	KBD_KEY_H,
	KBD_KEY_J,
	KBD_KEY_K,
	KBD_KEY_L,
	KBD_KEY_SEMICOLON,
	KBD_KEY_QUOTE,
	KBD_KEY_BACKQUOTE,
	KBD_KEY_LEFT_SHIFT,
	KBD_KEY_BACKSLASH,
	KBD_KEY_Z,
	KBD_KEY_X,
	KBD_KEY_C,
	KBD_KEY_V,
	KBD_KEY_B,
	KBD_KEY_N,
	KBD_KEY_M,
	KBD_KEY_COMMA,
	KBD_KEY_DOT,
	KBD_KEY_SLASH,
	KBD_KEY_RIGHT_SHIFT,
	KBD_KEY_PAD_ASTERISK,
	KBD_KEY_LEFT_ALT,
	KBD_KEY_SPACE,
	KBD_KEY_CAPSLOCK,
	KBD_KEY_F1,
	KBD_KEY_F2,
	KBD_KEY_F3,
	KBD_KEY_F4,
	KBD_KEY_F5,
	KBD_KEY_F6,
	KBD_KEY_F7,
	KBD_KEY_F8,
	KBD_KEY_F9,
	KBD_KEY_F10,
	KBD_KEY_F11,
	KBD_KEY_F12,
	KBD_KEY_NUMLOCK,
	KBD_KEY_SCROLLLOCK,
	KBD_KEY_PAD_0,
	KBD_KEY_PAD_1,
	KBD_KEY_PAD_2,
	KBD_KEY_PAD_3,
	KBD_KEY_PAD_4,
	KBD_KEY_PAD_5,
	KBD_KEY_PAD_6,
	KBD_KEY_PAD_7,
	KBD_KEY_PAD_8,
	KBD_KEY_PAD_9,
	KBD_KEY_PAD_PLUS,
	KBD_KEY_PAD_MINUS,
	KBD_KEY_PAD_DELETE,
	KBD_KEY_PAD_DOT,

};

enum kbd_act {
	KEY_PRESSED,
	KEY_RELEASED,
};

struct kbd_set_entry {
	uint8_t arr[KBD_BUF_MAX];
	enum kbd_key_enum val;
	enum kbd_act act;
};

struct kbd_set_entry kbd_set[3][0x100] = {
[KBD_SET_1] = {
	{ { 0x01, 0 },         KBD_KEY_ESCAPE,        KEY_PRESSED  },
	{ { 0x02, 0 },         KBD_KEY_1,             KEY_PRESSED  },
	{ { 0x03, 0 },         KBD_KEY_2,             KEY_PRESSED  },
	{ { 0x04, 0 },         KBD_KEY_3,             KEY_PRESSED  },
	{ { 0x05, 0 },         KBD_KEY_4,             KEY_PRESSED  },
	{ { 0x06, 0 },         KBD_KEY_5,             KEY_PRESSED  },
	{ { 0x07, 0 },         KBD_KEY_6,             KEY_PRESSED  },
	{ { 0x08, 0 },         KBD_KEY_7,             KEY_PRESSED  },
	{ { 0x09, 0 },         KBD_KEY_8,             KEY_PRESSED  },
	{ { 0x0a, 0 },         KBD_KEY_9,             KEY_PRESSED  },
	{ { 0x0b, 0 },         KBD_KEY_0,             KEY_PRESSED  },
	{ { 0x0c, 0 },         KBD_KEY_MINUS,         KEY_PRESSED  },
	{ { 0x0d, 0 },         KBD_KEY_EQUAL,         KEY_PRESSED  },
	{ { 0x0e, 0 },         KBD_KEY_BACKSPACE,     KEY_PRESSED  },
	{ { 0x0f, 0 },         KBD_KEY_TAB,           KEY_PRESSED  },
	{ { 0x10, 0 },         KBD_KEY_Q,             KEY_PRESSED  },
	{ { 0x11, 0 },         KBD_KEY_W,             KEY_PRESSED  },
	{ { 0x12, 0 },         KBD_KEY_E,             KEY_PRESSED  },
	{ { 0x13, 0 },         KBD_KEY_R,             KEY_PRESSED  },
	{ { 0x14, 0 },         KBD_KEY_T,             KEY_PRESSED  },
	{ { 0x15, 0 },         KBD_KEY_Y,             KEY_PRESSED  },
	{ { 0x16, 0 },         KBD_KEY_U,             KEY_PRESSED  },
	{ { 0x17, 0 },         KBD_KEY_I,             KEY_PRESSED  },
	{ { 0x18, 0 },         KBD_KEY_O,             KEY_PRESSED  },
	{ { 0x19, 0 },         KBD_KEY_P,             KEY_PRESSED  },
	{ { 0x1a, 0 },         KBD_KEY_OPEN_BRACKET,  KEY_PRESSED  },
	{ { 0x1b, 0 },         KBD_KEY_CLOSE_BRACKET, KEY_PRESSED  },
	{ { 0x1c, 0 },         KBD_KEY_ENTER,         KEY_PRESSED  },
	{ { 0x1d, 0 },         KBD_KEY_LEFT_CONTROL,  KEY_PRESSED  },
	{ { 0x1e, 0 },         KBD_KEY_A,             KEY_PRESSED  },
	{ { 0x1f, 0 },         KBD_KEY_S,             KEY_PRESSED  },
	{ { 0x20, 0 },         KBD_KEY_D,             KEY_PRESSED  },
	{ { 0x21, 0 },         KBD_KEY_F,             KEY_PRESSED  },
	{ { 0x22, 0 },         KBD_KEY_G,             KEY_PRESSED  },
	{ { 0x23, 0 },         KBD_KEY_H,             KEY_PRESSED  },
	{ { 0x24, 0 },         KBD_KEY_J,             KEY_PRESSED  },
	{ { 0x25, 0 },         KBD_KEY_K,             KEY_PRESSED  },
	{ { 0x26, 0 },         KBD_KEY_L,             KEY_PRESSED  },
	{ { 0x27, 0 },         KBD_KEY_SEMICOLON,     KEY_PRESSED  },
	{ { 0x28, 0 },         KBD_KEY_QUOTE,         KEY_PRESSED  },
	{ { 0x29, 0 },         KBD_KEY_BACKQUOTE,     KEY_PRESSED  },
	{ { 0x2a, 0 },         KBD_KEY_LEFT_SHIFT,    KEY_PRESSED  },
	{ { 0x2b, 0 },         KBD_KEY_BACKSLASH,     KEY_PRESSED  },
	{ { 0x2c, 0 },         KBD_KEY_Z,             KEY_PRESSED  },
	{ { 0x2d, 0 },         KBD_KEY_X,             KEY_PRESSED  },
	{ { 0x2e, 0 },         KBD_KEY_C,             KEY_PRESSED  },
	{ { 0x2f, 0 },         KBD_KEY_V,             KEY_PRESSED  },
	{ { 0x30, 0 },         KBD_KEY_B,             KEY_PRESSED  },
	{ { 0x31, 0 },         KBD_KEY_N,             KEY_PRESSED  },
	{ { 0x32, 0 },         KBD_KEY_M,             KEY_PRESSED  },
	{ { 0x33, 0 },         KBD_KEY_COMMA,         KEY_PRESSED  },
	{ { 0x34, 0 },         KBD_KEY_DOT,           KEY_PRESSED  },
	{ { 0x35, 0 },         KBD_KEY_SLASH,         KEY_PRESSED  },
	{ { 0x36, 0 },         KBD_KEY_RIGHT_SHIFT,   KEY_PRESSED  },
	{ { 0x37, 0 },         KBD_KEY_PAD_ASTERISK,  KEY_PRESSED  },
	{ { 0x38, 0 },         KBD_KEY_LEFT_ALT,      KEY_PRESSED  },
	{ { 0x39, 0 },         KBD_KEY_SPACE,         KEY_PRESSED  },
	{ { 0x3a, 0 },         KBD_KEY_CAPSLOCK,      KEY_PRESSED  },
	{ { 0x3b, 0 },         KBD_KEY_F1,            KEY_PRESSED  },
	{ { 0x3c, 0 },         KBD_KEY_F2,            KEY_PRESSED  },
	{ { 0x3d, 0 },         KBD_KEY_F3,            KEY_PRESSED  },
	{ { 0x3e, 0 },         KBD_KEY_F4,            KEY_PRESSED  },
	{ { 0x3f, 0 },         KBD_KEY_F5,            KEY_PRESSED  },
	{ { 0x40, 0 },         KBD_KEY_F6,            KEY_PRESSED  },
	{ { 0x41, 0 },         KBD_KEY_F7,            KEY_PRESSED  },
	{ { 0x42, 0 },         KBD_KEY_F8,            KEY_PRESSED  },
	{ { 0x43, 0 },         KBD_KEY_F9,            KEY_PRESSED  },
	{ { 0x44, 0 },         KBD_KEY_F10,           KEY_PRESSED  },
	{ { 0x45, 0 },         KBD_KEY_NUMLOCK,       KEY_PRESSED  },
	{ { 0x46, 0 },         KBD_KEY_SCROLLLOCK,    KEY_PRESSED  },
	{ { 0x47, 0 },         KBD_KEY_PAD_7,         KEY_PRESSED  },
	{ { 0x48, 0 },         KBD_KEY_PAD_8,         KEY_PRESSED  },
	{ { 0x49, 0 },         KBD_KEY_PAD_9,         KEY_PRESSED  },
	{ { 0x4a, 0 },         KBD_KEY_PAD_MINUS,     KEY_PRESSED  },
	{ { 0x4b, 0 },         KBD_KEY_PAD_4,         KEY_PRESSED  },
	{ { 0x4c, 0 },         KBD_KEY_PAD_5,         KEY_PRESSED  },
	{ { 0x4d, 0 },         KBD_KEY_PAD_6,         KEY_PRESSED  },
	{ { 0x4e, 0 },         KBD_KEY_PAD_PLUS,      KEY_PRESSED  },
	{ { 0x4f, 0 },         KBD_KEY_PAD_1,         KEY_PRESSED  },
	{ { 0x50, 0 },         KBD_KEY_PAD_2,         KEY_PRESSED  },
	{ { 0x51, 0 },         KBD_KEY_PAD_3,         KEY_PRESSED  },
	{ { 0x52, 0 },         KBD_KEY_PAD_0,         KEY_PRESSED  },
	{ { 0x53, 0 },         KBD_KEY_PAD_DOT,       KEY_PRESSED  },

	{ { 0x57, 0 },         KBD_KEY_F11,           KEY_PRESSED  },
	{ { 0x58, 0 },         KBD_KEY_F12,           KEY_PRESSED  },

	{ { 0x81, 0 },         KBD_KEY_ESCAPE,        KEY_RELEASED },
	{ { 0x82, 0 },         KBD_KEY_1,             KEY_RELEASED },
	{ { 0x83, 0 },         KBD_KEY_2,             KEY_RELEASED },
	{ { 0x84, 0 },         KBD_KEY_3,             KEY_RELEASED },
	{ { 0x85, 0 },         KBD_KEY_4,             KEY_RELEASED },
	{ { 0x86, 0 },         KBD_KEY_5,             KEY_RELEASED },
	{ { 0x87, 0 },         KBD_KEY_6,             KEY_RELEASED },
	{ { 0x88, 0 },         KBD_KEY_7,             KEY_RELEASED },
	{ { 0x89, 0 },         KBD_KEY_8,             KEY_RELEASED },
	{ { 0x8a, 0 },         KBD_KEY_9,             KEY_RELEASED },
	{ { 0x8b, 0 },         KBD_KEY_0,             KEY_RELEASED },
	{ { 0x8c, 0 },         KBD_KEY_MINUS,         KEY_RELEASED },
	{ { 0x8d, 0 },         KBD_KEY_EQUAL,         KEY_RELEASED },
	{ { 0x8e, 0 },         KBD_KEY_BACKSPACE,     KEY_RELEASED },
	{ { 0x8f, 0 },         KBD_KEY_TAB,           KEY_RELEASED },
	{ { 0x90, 0 },         KBD_KEY_Q,             KEY_RELEASED },
	{ { 0x91, 0 },         KBD_KEY_W,             KEY_RELEASED },
	{ { 0x92, 0 },         KBD_KEY_E,             KEY_RELEASED },
	{ { 0x93, 0 },         KBD_KEY_R,             KEY_RELEASED },
	{ { 0x94, 0 },         KBD_KEY_T,             KEY_RELEASED },
	{ { 0x95, 0 },         KBD_KEY_Y,             KEY_RELEASED },
	{ { 0x96, 0 },         KBD_KEY_U,             KEY_RELEASED },
	{ { 0x97, 0 },         KBD_KEY_I,             KEY_RELEASED },
	{ { 0x98, 0 },         KBD_KEY_O,             KEY_RELEASED },
	{ { 0x99, 0 },         KBD_KEY_P,             KEY_RELEASED },
	{ { 0x9a, 0 },         KBD_KEY_OPEN_BRACKET,  KEY_RELEASED },
	{ { 0x9b, 0 },         KBD_KEY_CLOSE_BRACKET, KEY_RELEASED },
	{ { 0x9c, 0 },         KBD_KEY_ENTER,         KEY_RELEASED },
	{ { 0x9d, 0 },         KBD_KEY_LEFT_CONTROL,  KEY_RELEASED },
	{ { 0x9e, 0 },         KBD_KEY_A,             KEY_RELEASED },
	{ { 0x9f, 0 },         KBD_KEY_S,             KEY_RELEASED },
	{ { 0xa0, 0 },         KBD_KEY_D,             KEY_RELEASED },
	{ { 0xa1, 0 },         KBD_KEY_F,             KEY_RELEASED },
	{ { 0xa2, 0 },         KBD_KEY_G,             KEY_RELEASED },
	{ { 0xa3, 0 },         KBD_KEY_H,             KEY_RELEASED },
	{ { 0xa4, 0 },         KBD_KEY_J,             KEY_RELEASED },
	{ { 0xa5, 0 },         KBD_KEY_K,             KEY_RELEASED },
	{ { 0xa6, 0 },         KBD_KEY_L,             KEY_RELEASED },
	{ { 0xa7, 0 },         KBD_KEY_SEMICOLON,     KEY_RELEASED },
	{ { 0xa8, 0 },         KBD_KEY_QUOTE,         KEY_RELEASED },
	{ { 0xa9, 0 },         KBD_KEY_BACKQUOTE,     KEY_RELEASED },
	{ { 0xaa, 0 },         KBD_KEY_LEFT_SHIFT,    KEY_RELEASED },
	{ { 0xab, 0 },         KBD_KEY_BACKSLASH,     KEY_RELEASED },
	{ { 0xac, 0 },         KBD_KEY_Z,             KEY_RELEASED },
	{ { 0xad, 0 },         KBD_KEY_X,             KEY_RELEASED },
	{ { 0xae, 0 },         KBD_KEY_C,             KEY_RELEASED },
	{ { 0xaf, 0 },         KBD_KEY_V,             KEY_RELEASED },
	{ { 0xb0, 0 },         KBD_KEY_B,             KEY_RELEASED },
	{ { 0xb1, 0 },         KBD_KEY_N,             KEY_RELEASED },
	{ { 0xb2, 0 },         KBD_KEY_M,             KEY_RELEASED },
	{ { 0xb3, 0 },         KBD_KEY_COMMA,         KEY_RELEASED },
	{ { 0xb4, 0 },         KBD_KEY_DOT,           KEY_RELEASED },
	{ { 0xb5, 0 },         KBD_KEY_SLASH,         KEY_RELEASED },
	{ { 0xb6, 0 },         KBD_KEY_RIGHT_SHIFT,   KEY_RELEASED },
	{ { 0xb7, 0 },         KBD_KEY_PAD_ASTERISK,  KEY_RELEASED },
	{ { 0xb8, 0 },         KBD_KEY_LEFT_ALT,      KEY_RELEASED },
	{ { 0xb9, 0 },         KBD_KEY_SPACE,         KEY_RELEASED },
	{ { 0xba, 0 },         KBD_KEY_CAPSLOCK,      KEY_RELEASED },
	{ { 0xbb, 0 },         KBD_KEY_F1,            KEY_RELEASED },
	{ { 0xbc, 0 },         KBD_KEY_F2,            KEY_RELEASED },
	{ { 0xbd, 0 },         KBD_KEY_F3,            KEY_RELEASED },
	{ { 0xbe, 0 },         KBD_KEY_F4,            KEY_RELEASED },
	{ { 0xbf, 0 },         KBD_KEY_F5,            KEY_RELEASED },
	{ { 0xc0, 0 },         KBD_KEY_F6,            KEY_RELEASED },
	{ { 0xc1, 0 },         KBD_KEY_F7,            KEY_RELEASED },
	{ { 0xc2, 0 },         KBD_KEY_F8,            KEY_RELEASED },
	{ { 0xc3, 0 },         KBD_KEY_F9,            KEY_RELEASED },
	{ { 0xc4, 0 },         KBD_KEY_F10,           KEY_RELEASED },
	{ { 0xc5, 0 },         KBD_KEY_NUMLOCK,       KEY_RELEASED },
	{ { 0xc6, 0 },         KBD_KEY_SCROLLLOCK,    KEY_RELEASED },
	{ { 0xc7, 0 },         KBD_KEY_PAD_7,         KEY_RELEASED },
	{ { 0xc8, 0 },         KBD_KEY_PAD_8,         KEY_RELEASED },
	{ { 0xc9, 0 },         KBD_KEY_PAD_9,         KEY_RELEASED },
	{ { 0xca, 0 },         KBD_KEY_PAD_MINUS,     KEY_RELEASED },
	{ { 0xcb, 0 },         KBD_KEY_PAD_4,         KEY_RELEASED },
	{ { 0xcc, 0 },         KBD_KEY_PAD_5,         KEY_RELEASED },
	{ { 0xcd, 0 },         KBD_KEY_PAD_6,         KEY_RELEASED },
	{ { 0xce, 0 },         KBD_KEY_PAD_PLUS,      KEY_RELEASED },
	{ { 0xcf, 0 },         KBD_KEY_PAD_1,         KEY_RELEASED },
	{ { 0xd0, 0 },         KBD_KEY_PAD_2,         KEY_RELEASED },
	{ { 0xd1, 0 },         KBD_KEY_PAD_3,         KEY_RELEASED },
	{ { 0xd2, 0 },         KBD_KEY_PAD_0,         KEY_RELEASED },
	{ { 0xd3, 0 },         KBD_KEY_PAD_DOT,       KEY_RELEASED },

	{ { 0xd7, 0 },         KBD_KEY_F11,           KEY_RELEASED },
	{ { 0xd8, 0 },         KBD_KEY_F12,           KEY_RELEASED },

	/* Multimedia */
//	{ { 0xe0, 0 },         KBD_KEY_F12,           KEY_RELEASED },
//	{ { 0xe0, 0 },         KBD_KEY_F12,           KEY_RELEASED },
}
};


#endif /* _KBD_SETS_H_ */
