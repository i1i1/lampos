#include "kernel.h"
#include "vga.h"
#include "com.h"
#include "defs.h"
#include "mb_parce.h"
#include "pgalloc.h"
#include "interrupt.h"
#include "buddyalloc.h"

#define VERSION_MAJOR	0
#define VERSION_MINOR	1


extern char etext, edata, end;
extern struct mb_header multiboot;

extern void segm_init();


void
main(size_t cr0, struct mb_info *mb)
{
	int mmlen;
	struct mm_area **mm;

	cli();

	vga_init();
	com_init(COM1_PORT_ADDRESS);
	com_init(COM2_PORT_ADDRESS);
	segm_init();
	int_init();

	iprintf("\tmb = %p\n", mb);

	mb_parse(mb, &mm, &mmlen);
	pginit(mm, mmlen);

	for (;;);

	/* Allocating 8 * 1024 Pages(32 MB total) to allocator */
	balloc_init(1);
	//balloc_info();

	iprintf("\ttext ends at 0x%08x\n", &etext);
	iprintf("\tdata ends at 0x%08x\n", &edata);
	iprintf("\tbss and kernel ends at 0x%08x\n", &end);

/*
	char c;

	iprintf("COM2 DATA BEGIN:\n");

	do {
		c = com_getc(COM2_PORT_ADDRESS);
		iprintf("%c", c);
	} while(c != '\0');

	iprintf("\nCOM2 DATA END\n");
*/

//	iprintf("LampOS v%u.%02u\n", VERSION_MAJOR, VERSION_MINOR);
//	iprintf("x86-32 version, uniprocessor kernel\n");
//	iprintf("\n\tHello, kernel world!\n\n");

	vga_move_hardware_cursor(81, 25);
}

