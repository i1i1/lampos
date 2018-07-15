#include "defs.h"
#include "pic.h"
#include "pit.h"
#include "kernel.h"
#include "interrupt.h"
#include "vga.h"

#include "keyboard.h"
#include "kbd_sets.h"

extern void kbd_asm_handler();
extern void ps_2_asm_handler();

typedef unsigned char kbd_code;


static volatile char key_buf[KEYBUFSZ];
static volatile int beg = 0;
static volatile int end = 0;

static kbd_type kbd_cur_set;

static int shift = 0;

kbd_code_type
kbd_get_type_code(kbd_code c)
{
	return kbd_set[kbd_cur_set][c].tp;
}

char
kbd_get_char_code(kbd_code c)
{
	return kbd_set[kbd_cur_set][c].c;
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

void
kbd_irq()
{
	kbd_code c;
	kbd_code_type tp;
	char chr;
//	kbd_special_type stp;

	c = inb(PS_2_DATA);
	tp = kbd_get_type_code(c);
	chr = kbd_get_char_code(c);

	/* Should be adding character to the key_buf */
	switch (tp) {
	case KBD_PRESSED:
		//dprintf("Key '%c' pressed!\n", kbd_get_char_code(c));
		if (shift)
			chr = upcase(chr);
		buf_putc(chr);
		vga_putc(chr);
		break;
	case KBD_RELEASED:
		//dprintf("Key '%c' released!\n", kbd_get_char_code(c));
		break;
	case KBD_PRESSED_SPECIAL:
		switch (chr) {
		case '\b':
		case '\n':
		case ' ':
			buf_putc(chr);
			vga_putc(chr);
			break;
		case 's':
			shift = 1;
			break;
		case '-':
			vga_screen_scroll_down();
			break;
		case '+':
			vga_screen_scroll_up();
			break;
		default:
			break;
		}
		break;
	case KBD_RELEASED_SPECIAL:
		switch (chr) {
		case 's':
			shift = 0;
			break;
		default:
			break;
		}
	default:
		break;
	}

	if (end == NELEMS(key_buf))
		end = 0;

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
	outb(PS_2_DATA, conf & 0b10111100);

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
}

