#ifndef _VGA_H_
#define _VGA_H_

/* VGA text mode palette colors */

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


void vga_init();

void vga_move_hardware_cursor(int x, int y);

void vga_putc(int c);

void vga_puts(const char *s);

void vga_set_bgcolor(int color);

void vga_set_fgcolor(int color);

void vga_get_cursor(int *x, int *y);

int vga_get_bgcolor();

int vga_get_fgcolor();


int vga_history_init();

void vga_history_up();

void vga_history_down();


#endif

