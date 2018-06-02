/* Howto use PIC tuto: http://www.brokenthorn.com/Resources/OSDevPic.html */
#include <stdint.h>

#include "pic.h"
#include "kernel.h"

enum pic_icw {
	ICW_INIT	= 0x10,
	ICW_IC4		= 0x01,
};

enum {
	IRQ_TIMER_MASK	= 1,
	IRQ_KBD_MASK	= 2,
};

// choose which interrupts we want to recieve
#define IRQ_ALL 0x3	//IRQ_TIMER | IRQ_KBD

static inline void pic_set_imr(enum pic_port p, uint8_t mask);

static inline void io_wait(void)
{
	asm volatile ( "movb $0,  %%al" : : "a"(0) );
	asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

void
pic_init(void)
{
	// ICW 1. Initialisation.
	outb(PIC_MASTER_CMD, ICW_INIT | ICW_IC4);
	outb(PIC_SLAVE_CMD, ICW_INIT | ICW_IC4);
	io_wait();

	// ICW 2. Map interrupts.
	outb(PIC_MASTER_DT, _PIC_INT_OFF1);
	outb(PIC_SLAVE_DT, _PIC_INT_OFF2);
	io_wait();

	// ICW 3. Connect slave with master.
	outb(PIC_MASTER_DT, 0x4);	// 0x4 -- IRQ2 pin
	outb(PIC_SLAVE_DT, 0x2);	// 0x4 -- IRQ2 pin address
					// in binary notation
	io_wait();

	// ICW4. final steps
	outb(PIC_MASTER_DT, 0x1);	// Enable PIC for 80x86 mode
	io_wait();

	pic_set_imr(PIC_MASTER_DT, IRQ_ALL);
}

// Sets mask register (IMR)
static inline void
pic_set_imr(enum pic_port p, uint8_t mask)
{
	//assert(p == PIC_MASTER_DT || p == PIC_SLAVE_DT);
	outb(p, ~mask);
}

// Send end of interrupt to primary PIC
void
pic_eoi(enum pic_port p)
{
	if (p == PIC_SLAVE_CMD)
		outb(PIC_SLAVE_CMD, 0x20);
	outb(PIC_MASTER_CMD, 0x20);
}

