#include "stdint.h"

#include "kernel.h"
#include "vga.h"
#include "com.h"
#include "defs.h"
#include "mb_parce.h"

#define VERSION_MAJOR	0
#define VERSION_MINOR	1


extern void segm_init();


void
main(size_t eax, void *multiboot)
{
	cli();

	vga_init();
	com_init(COM1_PORT_ADDRESS);
	com_init(COM2_PORT_ADDRESS);
	segm_init();
	mb_parse(multiboot);

#if 0
	char c;

	iprintf("COM2 DATA BEGIN:\n");

	do {
		c = com_getc(COM2_PORT_ADDRESS);
		iprintf("%c", c);
	} while(c != '\0');

	iprintf("\nCOM2 DATA END\n");
#endif

//	iprintf("LampOS v%u.%02u\n", VERSION_MAJOR, VERSION_MINOR);
//	iprintf("x86-32 version, uniprocessor kernel\n");
//	iprintf("\n\tHello, kernel world!\n\n");

	vga_move_hardware_cursor(81, 25);
}

