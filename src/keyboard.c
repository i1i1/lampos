#include "defs.h"
#include "pic.h"
#include "pit.h"
#include "kernel.h"
#include "interrupt.h"
#include "vga.h"

#include "keyboard.h"
#include "kbd_sets.h"

extern void kbd_asm_handler();

typedef unsigned char kbd_code;


static volatile char key_buf[KEYBUFSZ];
static volatile int beg = 0;
static volatile int end = 0;

static uint8_t key[KBD_BUF_MAX];
static int key_idx;

static kbd_type kbd_cur_set;

static int shift = 0;
static int cntrl = 0;


char
kbd_get_char_code(enum kbd_key_enum c)
{
	switch (c) {
	case KBD_KEY_A:				return 'a';
	case KBD_KEY_B:				return 'b';
	case KBD_KEY_C:				return 'c';
	case KBD_KEY_D:				return 'd';
	case KBD_KEY_E:				return 'e';
	case KBD_KEY_F:				return 'f';
	case KBD_KEY_G:				return 'g';
	case KBD_KEY_H:				return 'h';
	case KBD_KEY_I:				return 'i';
	case KBD_KEY_J:				return 'j';
	case KBD_KEY_K:				return 'k';
	case KBD_KEY_L:				return 'l';
	case KBD_KEY_M:				return 'm';
	case KBD_KEY_O:				return 'o';
	case KBD_KEY_P:				return 'p';
	case KBD_KEY_Q:				return 'q';
	case KBD_KEY_R:				return 'r';
	case KBD_KEY_S:				return 's';
	case KBD_KEY_T:				return 't';
	case KBD_KEY_U:				return 'u';
	case KBD_KEY_V:				return 'v';
	case KBD_KEY_W:				return 'w';
	case KBD_KEY_X:				return 'x';
	case KBD_KEY_Y:				return 'y';
	case KBD_KEY_Z:				return 'z';
	case KBD_KEY_0:				return '0';
	case KBD_KEY_1:				return '1';
	case KBD_KEY_2:				return '2';
	case KBD_KEY_3:				return '3';
	case KBD_KEY_4:				return '4';
	case KBD_KEY_5:				return '5';
	case KBD_KEY_6:				return '6';
	case KBD_KEY_7:				return '7';
	case KBD_KEY_8:				return '8';
	case KBD_KEY_9:				return '9';
	case KBD_KEY_MINUS:			return '-';
	case KBD_KEY_EQUAL:			return '=';
	case KBD_KEY_SLASH:			return '/';
	case KBD_KEY_BACKSLASH:		return '\\';
	case KBD_KEY_DOT:			return '.';
	case KBD_KEY_COMMA:			return ',';
	case KBD_KEY_ENTER:			return '\n';
	case KBD_KEY_SPACE:			return ' ';
	case KBD_KEY_TAB:			return '\t';
	case KBD_KEY_BACKSPACE:		return '\b';
	case KBD_KEY_OPEN_BRACKET:	return '[';
	case KBD_KEY_CLOSE_BRACKET:	return ']';
	case KBD_KEY_SEMICOLON:		return ';';
	case KBD_KEY_QUOTE:			return '\'';

	default:					return '\0';
	}
}

char
upcase(char c)
{
	if ('a' <= c && c <= 'z')
		return c - 0x20;

#define cs(c1, c2) case c1: return c2
	switch (c) {
		cs('1', '!');
		cs('2', '@');
		cs('3', '#');
		cs('4', '$');
		cs('5', '%');
		cs('6', '^');
		cs('7', '&');
		cs('8', '*');
		cs('9', '(');
		cs('0', ')');
		cs('-', '_');
		cs('=', '+');
		cs('`', '~');
		cs('[', '{');
		cs(']', '}');
		cs('\\', '|');
		cs(';', ':');
		cs('\'', '"');
		cs(',', '<');
		cs('.', '>');
		cs('/', '?');
	default:
		return c;
	}
#undef cs
}

char
buf_getc()
{
	char c;

	/* If number of chars in buf == 0 */
	if (end == beg)
		return '\0';

	c = key_buf[beg];
	beg = (beg + 1) % NELEMS(key_buf);

	return c;
}

void
buf_putc(char c)
{
	key_buf[end] = c;
	end = (end + 1) % NELEMS(key_buf);

	/* If number of chars in buf == NELEMS(key_buf) */
	if (beg == end)
		beg = (beg + 1) % NELEMS(key_buf);
}

int
kbd_set_lookup(struct kbd_set_entry **ret, uint8_t *key)
{
	struct kbd_set_entry *set;
	int i, j;

	set = kbd_set[kbd_cur_set];

	for (i = 0; i < 0x100; i++) {
		for (j = 0; j < KBD_BUF_MAX; j++) {
			if (set[i].arr[j] == 0) {
				*ret = set + i;
				return 0;
			}
			if (set[i].arr[j] != key[j])
				break;
		}
	}
	return 1;
}

void
kbd_irq()
{
	struct kbd_set_entry *kbd_entry;
	char chr;

	key[key_idx++] = inb(PS_2_DATA);

	if (kbd_set_lookup(&kbd_entry, key)) {
		pic_eoi(PIC_MASTER_CMD);
		return;
	}

	chr = kbd_get_char_code(kbd_entry->val);

	if (kbd_entry->act == KEY_PRESSED) {
		switch (kbd_entry->val) {
		case KBD_KEY_LEFT_SHIFT:
//		case KBD_KEY_RIGHT_SHIFT:
			shift = 1;
			break;
		case KBD_KEY_LEFT_CONTROL:
//		case KBD_KEY_RIGHT_CONTROL:
			cntrl = 1;
			break;
		case KBD_KEY_J:
			if (cntrl) {
				vga_history_up();
				break;
			}
		case KBD_KEY_K:
			if (cntrl) {
				vga_history_down();
				break;
			}
		default:
			if (shift)
				chr = upcase(chr);

			buf_putc(chr);
			break;
		}
	} else {
		switch (kbd_entry->val) {
		case KBD_KEY_LEFT_SHIFT:
//		case KBD_KEY_RIGHT_SHIFT:
			shift = 0;
			break;
		case KBD_KEY_LEFT_CONTROL:
//		case KBD_KEY_RIGHT_CONTROL:
			cntrl = 0;
			break;
		default:
			//dprintf("Key '%c' released!\n", kbd_get_char_code(c));
			break;
		}
	}

	if (end == NELEMS(key_buf))
		end = 0;
	memset(key, 0, sizeof(key));
	key_idx = 0;

	pic_eoi(PIC_MASTER_CMD);
}

int
getchar()
{
	while (beg == end)
		pit_sleep(50);

	return buf_getc();
}

void
ungetchar(char c)
{
	buf_putc(c);
}

void
ps_2_output_wait()
{
	while (!(inb(PS_2_STATUS) & (1 << 0)))
		;
}

void
ps_2_input_wait()
{
	while (inb(PS_2_STATUS) & (1 << 1))
		;
}

void
ps_2_outb(uint8_t cmd)
{
	ps_2_input_wait();
	outb(PS_2_COMMAND, cmd);
}

uint8_t
ps_2_inb()
{
	ps_2_output_wait();
	return inb(PS_2_DATA);
}

int
ps_2_dev1_outb(uint8_t b)
{
	size_t st, ct;

	st = pit_get_ticks();

	while (inb(PS_2_STATUS) & (1 << 1)) {
		ct = pit_get_ticks();
		/* Ur time is up! */
		if (ct - st > 100)
			return 1;
	}

	outb(PS_2_DATA, b);

	return 0;
}

uint8_t
ps_2_dev1_inb()
{
	return ps_2_inb();
}

void
ps_2_ack()
{
	assert_or_panic(ps_2_inb() == KBD_ACK, "ps_2_ack failed");
}

void kbd_init();

/* Inits first ps/2 */
void
ps_2_init()
{
	uint8_t conf, dev_type;

	/* Disabling PS/2 ports */
	ps_2_outb(0xAD);
	ps_2_outb(0xA7);

	/* Flushing buffer */
	while (inb(PS_2_STATUS) & 1)
		inb(PS_2_DATA);

	/* Disabling interrupts and translation */
	ps_2_outb(0x20);
	conf = ps_2_inb();
	ps_2_outb(0x60);
	outb(PS_2_DATA, conf & 0xbc); // 0b10111100

	/* Controller self test */
	ps_2_outb(0xAA);
	assert_or_panic(ps_2_inb() == 0x55, "Controller self test failed");

	/* First PS/2 port test */
	ps_2_outb(0xAB);
	assert_or_panic(ps_2_inb() == 0x00, "First PS/2 port test failed");

	/* Enabling translation and IRQ */
	ps_2_outb(0x60);
	outb(PS_2_DATA, 0x71);

	/* Enabling first PS/2 port */
	ps_2_outb(0xAE);

	/* Resetting first PS/2 port */
	/*
	 * Idk why, but dev1 sends this byte.
	 * Probably OSDEV doesn't cover all
	 *
	 * Here it is covered:
	 * http://www.flingos.co.uk/docs/reference/PS2-Keyboards/#implementation-details
	 */
	ps_2_dev1_outb(0xFF);
	ps_2_ack();
	assert_or_panic(ps_2_inb() == PS_2_ST_PASSED,
			"Device self test failed");

	/* Disable scanning */
	ps_2_input_wait();
	assert_or_panic(ps_2_dev1_outb(0xF5) == 0, "outb to dev1 failed");
	ps_2_ack();

	/* Identifying device */
	assert_or_panic(ps_2_dev1_outb(0xF2) == 0, "outb to dev1 failed");
	ps_2_ack();

	dev_type = ps_2_dev1_inb();

	switch (dev_type) {
	case 0x00:
		dprintf("Dats Standart Mouse!\n");
		break;
	case 0x03:
		dprintf("Dats Standart Mouse with Scroll wheel!\n");
		break;
	case 0x04:
		dprintf("Dats Standart Mouse with 5 buttons!\n");
		break;
	case 0xAB:
		dev_type = ps_2_dev1_inb();

		switch (dev_type) {
			case 0x41:
			case 0xAB:
			case 0xC1:
				dprintf("Dats MF2 keyboard with translation!\n");
				break;
			case 0x83:
				dprintf("Dats MF2 keyboard!\n");
				break;
			default:
				panic("Unknown type of PS/2 device");
		}

		kbd_init();
		break;
	default:
		panic("Unknown type of PS/2 device");
	}

	/* Enable scanning */
	assert_or_panic(ps_2_dev1_outb(0xF4) == 0, "outb to dev1 failed");
	ps_2_ack();

	int_add(IRQ_PS_2, 1, TRAP_GATE, 0, kbd_asm_handler);
	pic_imr_add(IRQ_PS_2_MASK);
}

void
kbd_init()
{
	kbd_cur_set = KBD_SET_1;

	/* Unsetting leds on keyboard */
	ps_2_dev1_outb(0xED);
	ps_2_dev1_outb(0);

	/* Switching keyboard to kbd_cur_set */
	ps_2_dev1_outb(0xF0);
	ps_2_dev1_outb(kbd_cur_set);
	ps_2_ack();

	memset(key, 0, sizeof(key));
	key_idx = 0;
}

