#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define KEYBUFSZ	100

typedef enum {
	KBD_ACK		= 0xFA,
	KBD_RESEND	= 0xFE,
	KBD_ECHO	= 0xEE,
	PS_2_ST_PASSED	= 0xAA,
} kbd_answer;

typedef enum {
	KBD_SET_1 = 0,
	KBD_SET_2 = 1,
	KBD_SET_3 = 2,
} kbd_type;

typedef enum {
	KBD_PRESSED,
	KBD_RELEASED,
	KBD_PRESSED_SPECIAL,
	KBD_RELEASED_SPECIAL,
	KBD_INVALID,
} kbd_code_type;

typedef enum {
	PS_2_DATA = 0x60,
	PS_2_COMMAND = 0x64,
	PS_2_STATUS = 0x64,
} ps_2_registers;


/* DO NOT CALL! */
void kbd_irq();

int getchar();
void ungetchar(char c);
void ps_2_init();

#endif /* _KEYBOARD_H_ */
