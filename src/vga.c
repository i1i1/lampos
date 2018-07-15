#include "kernel.h"
#include "buddyalloc.h"
#include "vga.h"
#include "pit.h"

/* VGA registers accessed through port `base_port'. */
#define VGA_CURSOR_MSB		0x0E
#define VGA_CURSOR_LSB		0x0F

/* VGA palette colors */
#define COLOR_BRIGHT		0x8

#define COLOR_BLACK		0x0
#define COLOR_BLUE		0x1
#define COLOR_GREEN		0x2
#define COLOR_CYAN		0x3
#define COLOR_RED		0x4
#define COLOR_MAGENTA		0x5
#define COLOR_BROWN		0x6
#define COLOR_LIGHT_GREY	0x7
#define COLOR_DARK_GREY		(COLOR_BLACK | COLOR_BRIGHT)
#define COLOR_LIGHT_BLUE	(COLOR_BLUE | COLOR_BRIGHT)
#define COLOR_LIGHT_CYAN	(COLOR_CYAN | COLOR_BRIGHT)
#define COLOR_LIGHT_RED		(COLOR_RED | COLOR_BRIGHT)
#define COLOR_LIGHT_MAGENTA	(COLOR_MAGENTA | COLOR_BRIGHT)
#define COLOR_YELLOW		(COLOR_BROWN | COLOR_BRIGHT)
#define COLOR_WHITE		(COLOR_LIGHT_GREY | COLOR_BRIGHT)

#define MAX_HISTORY_LINES	200

static unsigned int base_port;
static uint16_t *buffer = NULL;
static unsigned int width, height;
static int x_coord = 0, y_coord = 0;
static int bg_color = COLOR_BLACK, fg_color = COLOR_LIGHT_GREY;
static uint32_t tabs[8];
static int crlf = 0;

int max_top_screen_line = 0;
int top_screen_line = 0;
uint16_t *history = NULL;
int history_enabled = 0;


static unsigned int
bios_get_param(unsigned long addr, unsigned int len)
{
	uint8_t *mem;
	unsigned int par;

	par = 0;
	mem = (uint8_t *)addr;

	switch (len) {
	case 1: par = mem[0]; break;
	case 2: par = mem[0] | mem[1] << 8; break;
	case 4: par = mem[0] | mem[1] << 8 | mem[2] << 16 | mem[3] << 24; break;
	}

	return par;
}

static void
vga_get_params(unsigned int *width, unsigned int *height, unsigned int *base_port)
{
	if (base_port != NULL)
		*base_port = bios_get_param(BDA_BASE_PORT_ADDRESS, 2);
	if (width != NULL)
		*width = bios_get_param(BDA_NUM_COLUMNS, BDA_NUM_COLUMNS_LEN);
	if (height != NULL)
		*height = bios_get_param(BDA_NUM_ROWS, BDA_NUM_ROWS_LEN);
}

/* This function moves hardware cursor to <x,y> position. */
void
vga_move_hardware_cursor(int x, int y)
{
	unsigned int pos;

	pos = y*width + x;

	outb(base_port, VGA_CURSOR_LSB);
	outb(base_port+1, pos & 0xFF);
	outb(base_port, VGA_CURSOR_MSB);
	outb(base_port+1, (pos >> 8) & 0xFF);
}

/* This function constructs VGA text buffer character from its attributes. */
static uint16_t
vga_make_char(int c, int fg_color, int bg_color)
{
	uint16_t res;

	res = ((fg_color & 0xF) | (bg_color & 0xF) << 4) << 8;
	res |= (unsigned char)c;

	return res;
}

/* This is a helper to set foreground color. */
void
vga_set_fgcolor(int color)
{
	fg_color = color & 0xF;
}

/* This is a helper to set background color. */
void
vga_set_bgcolor(int color)
{
	bg_color = color & 0xF;
}

/* This function clears the screen using current background/foreground color. */
/* Also clears history. */
void
vga_clear_screen()
{
	int x, y;

	if (buffer == NULL)
		return;

	x_coord = 0;
	y_coord = 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] =
				vga_make_char(' ', fg_color, bg_color);
		}
	}

	if (history_enabled)
		memset(history, 0,
		       sizeof(uint16_t) * MAX_HISTORY_LINES * width);
}

void
vga_history_init()
{
	history = balloc(width * height * sizeof(uint16_t));

	if (history) {
		history_enabled = 1;
		vga_clear_screen();
		dprintf("History should work now!\n");
	}
}

void
vga_init()
{
	int i;

	buffer = (void *)VGA_BUFFER_ADDR;

	/* Get CRT controller parameters from BIOS Data Area. */
	vga_get_params(&width, &height, &base_port);

	height++;

	/* Initialize default settings. */
	x_coord = 0;
	y_coord = 0;
	crlf = 1;
	vga_set_bgcolor(COLOR_BLACK);
	vga_set_fgcolor(COLOR_GREEN);

	/* Clear screen with default colors. */
	vga_clear_screen();

	/* Set TAB stops after each 8 characters. */
	for (i = 0; i < NELEMS(tabs); i++)
		tabs[i] = 0x01010101;

	/* Move hardware cursor to upper left corner. */
	vga_move_hardware_cursor(0, 0);
}

static void
vga_scroll_up_old()
{
	int x, y;
 
	for (y = 0; y < height-1; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] = buffer[(y+1)*width + x];
		}
	}

	for (x = 0; x < width; x++) {
		buffer[y*width + x] =
			vga_make_char(' ', fg_color, bg_color);
	}
}

static void
vga_scroll_up()
{
	int x, y;

	if (max_top_screen_line == MAX_HISTORY_LINES - height) {
		for (y = 0; y < max_top_screen_line-1; y++) {
			for (x = 0; x < width; x++) {
				history[y*width + x] =
					history[(y+1)*width + x];
			}
		}
		max_top_screen_line--;
		top_screen_line--;
	}

	/* Save top line in history */
	for (x = 0; x < width; x++) {
		history[top_screen_line*width + x] = buffer[x];
	}

	for (y = 0; y < height-1; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] = buffer[(y+1)*width + x];
		}
	}

	for (x = 0; x < width; x++) {
		buffer[y*width + x] =
			vga_make_char(' ', fg_color, bg_color);
	}

	max_top_screen_line++;
	top_screen_line++;
}

void
vga_screen_scroll_up()
{
	int x, y;

	if (max_top_screen_line == top_screen_line)
		return;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] =
				history[(top_screen_line+y+1)*width + x];
		}
	}

	top_screen_line++;
}

void
vga_screen_scroll_down()
{
	int x, y;

	if (top_screen_line == 0)
		return;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] =
				history[(top_screen_line+y-1)*width + x];
		}
	}

	top_screen_line--;
}

void
vga_reset_screen()
{
	int x, y;

	if (top_screen_line == max_top_screen_line)
		return;

	top_screen_line = max_top_screen_line;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] =
				history[(top_screen_line + y)*width + x];
		}
	}
}

void
vga_putc(int c)
{
	uint16_t chr;
	int x;

	if (buffer == NULL)
		return;

	vga_reset_screen();

	switch (c) {
	case '\a':
	case '\v':
		break;
	case '\b':
		/* Erase one character after cursor position. */
		if (x_coord > 0) {
			x_coord--;
			buffer[width*y_coord + x_coord] =
				vga_make_char(' ', fg_color, bg_color);
			if (history_enabled)
				history[(top_screen_line+y_coord)*width + x_coord] =
					vga_make_char(' ', fg_color, bg_color);
		}
		break;
	case '\t':
		/* Locate next TAB position and move to it. */
		for (x = x_coord+1; x < width; x++) {
			if (tabs[x/32] & (1 << (x%32))) {
				x_coord = x;
				break;
			}
		}
		break;
	case '\n':
		if (crlf)
			x_coord = 0;
		y_coord++;
		break;
	case '\f':
		vga_clear_screen();
		break;
	case '\r':
		x_coord = 0;
		break;
	default:
		/* Put printable character to screen. */
		chr = vga_make_char(c, fg_color, bg_color);
		buffer[width*y_coord + x_coord] = chr;
		if (history_enabled)
			history[(top_screen_line+y_coord)*width + x_coord] = chr;
		x_coord++;
		break;
	}

	if (x_coord >= width) {
		x_coord = 0;
		y_coord++;
	}
	if (y_coord >= height) {
		if (history_enabled)
			vga_scroll_up();
		else
			vga_scroll_up_old();
		y_coord--;
	}

	vga_move_hardware_cursor(x_coord, y_coord);
}

void
vga_puts(const char *s)
{
	while (*s != '\0')
		vga_putc(*s++);
}

