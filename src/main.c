#include "kernel.h"
#include "vga.h"
#include "com.h"
#include "defs.h"
#include "mb_parce.h"
#include "pic.h"
#include "pgalloc.h"
#include "interrupt.h"
#include "buddyalloc.h"
#include "keyboard.h"
#include "pit.h"
#include "pci.h"
#include "uhci.h"
#include "shell.h"

#define VERSION_MAJOR	0
#define VERSION_MINOR	2


extern char etext, edata, end;
extern struct mb_header multiboot;

extern void segm_init();


void
main(size_t cr0, struct mb_info *mb)
{
	int mmlen;
	struct mm_area **mm;

	assert_or_panic(mb, "Multiboot structure points to NULL");

	com_init(COM1_PORT_ADDRESS);
//	com_init(COM2_PORT_ADDRESS);
	vga_init();

	dprintf("\tmb = %p\n", mb);
	dprintf("\tend = %p\n", &end);
	dprintf("\ttext ends at 0x%08x\n", &etext);
	dprintf("\tdata ends at 0x%08x\n", &edata);
	dprintf("\tbss and kernel ends at 0x%08x\n\n", &end);

	pic_init();
	int_init();

	/* Memory init */
	segm_init();
	mb_parse(mb, &mm, &mmlen);
	mem_init(mm, mmlen);
	vga_history_init();

	pit_init();
	ps_2_init();

	pci_init();
//	uhci_init();

	iprintf("\n\nLampOS v%u.%02u\n", VERSION_MAJOR, VERSION_MINOR);
	iprintf("x86-32 version, uniprocessor kernel\n");
	iprintf("\n\tHello, kernel world!\n\n");

	shell();

	for (;;)
		halt();
}

