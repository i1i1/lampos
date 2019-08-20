#include "kernel.h"
#include "buddyalloc.h"
#include "vga.h"

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


#define HISTORY_LINES	200


static unsigned int base_port;
static uint16_t *buffer = NULL;
static unsigned int width, height;
static int x_coord = 0, y_coord = 0;
static int bg_color = COLOR_BLACK, fg_color = COLOR_LIGHT_GREY;
static uint32_t tabs[8];
static int crlf = 0;


char *history = NULL;
int history_enabled = 0;
int history_top_line;
int history_screen_line;
int history_reset_line;

/* For optimization of vga.c:/^vga_history_update */
int history_last_line;


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
	for (i = 0; i < ARRAY_SIZE(tabs); i++)
		tabs[i] = 0x01010101;

	/* Move hardware cursor to upper left corner. */
	vga_move_hardware_cursor(0, 0);
}

static void
vga_history_scroll_up()
{
	int hist_y;

	if (!history || !history_enabled)
		return;

	hist_y = (history_reset_line + height) % HISTORY_LINES;
	memset(history + hist_y*width, ' ', width);

	history_screen_line = (history_screen_line + 1) % HISTORY_LINES;
	history_reset_line = history_screen_line;

	if ((history_reset_line + height - 1) % HISTORY_LINES
	    == history_top_line)
		history_top_line = (history_top_line + 1) % HISTORY_LINES;
}

static void
vga_history_update()
{
	int x, y, hist_y;
	char c;
	uint16_t chr;

	if (!history || !history_enabled)
		return;

	if (history_last_line == history_screen_line)
		return;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			hist_y = (history_screen_line+y) % HISTORY_LINES;
			c = history[hist_y * width + x];
			chr = vga_make_char(c, fg_color, bg_color);
			buffer[y * width + x] = chr;
		}
	}

	history_last_line = history_screen_line;
}

void
vga_history_up()
{
	if (!history || !history_enabled)
		return;

	if (history_reset_line == history_screen_line)
		return;

	history_screen_line = (history_screen_line + 1) % HISTORY_LINES;

	vga_history_update();
}

void
vga_history_down()
{
	if (!history || !history_enabled)
		return;

	if (history_screen_line == history_top_line)
		return;

	history_screen_line = (history_screen_line - 1) % HISTORY_LINES;

	vga_history_update();
}

static void
vga_history_clear()
{
	if (!history || !history_enabled)
		return;

	history_screen_line = 0;
	history_reset_line = 0;
	history_top_line = 0;

	memset(history, ' ', HISTORY_LINES * width);
}

int
vga_history_init()
{
	if (history_enabled)
		return 0;

	history = balloc(HISTORY_LINES * width);

	if (!history)
		return 1;

	history_enabled = 1;

	vga_clear_screen();
	vga_history_clear();

	return 0;
}

static void
vga_scroll_up()
{
	int x, y;

	for (y = 0; y < height-1; y++) {
		for (x = 0; x < width; x++) {
			buffer[y*width + x] = buffer[(y+1)*width + x];
		}
	}

	for (x = 0; x < width; x++)
		buffer[y*width + x] = vga_make_char(' ', fg_color, bg_color);
}

void
vga_putc(int c)
{
	uint16_t chr;
	int x;

	if (buffer == NULL)
		return;

	if (history_enabled) {
		history_screen_line = history_reset_line;
		vga_history_update();
	}

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
			history[(history_screen_line+y_coord)*width + x_coord] = ' ';
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
		if (history_enabled)
			vga_history_clear();
		break;
	case '\r':
		x_coord = 0;
		break;
	default:
		/* Put printable character to screen. */
		chr = vga_make_char(c, fg_color, bg_color);
		buffer[width*y_coord + x_coord] = chr;
		if (history_enabled && history) {
			int hist_y;
			hist_y = (history_screen_line+y_coord) % HISTORY_LINES;
			history[hist_y * width + x_coord] = c;
		}
		x_coord++;
		break;
	}

	if (x_coord >= width) {
		x_coord = 0;
		y_coord++;
	}
	if (y_coord >= height) {
		if (history_enabled)
			vga_history_scroll_up();
		else
			vga_scroll_up();
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

