/* Howto use PIC tuto: http://www.brokenthorn.com/Resources/OSDevPic.html */
#include <stdint.h>

#include "pic.h"
#include "kernel.h"

enum pic_icw {
	ICW_INIT	= 0x10,
	ICW_IC4		= 0x01,
};


static uint16_t pic_imr;

static inline void pic_set_imr(enum pic_port p, uint8_t mask);

extern void io_wait(void);

void
pic_imr_add(enum pic_imr_off off)
{
	pic_imr |= (1 << off);

	if (off < 8)
		pic_set_imr(PIC_MASTER_DT, (uint8_t)pic_imr);
	else
		pic_set_imr(PIC_MASTER_DT, (uint8_t)(pic_imr >> 8));
}

void
pic_init()
{
	pic_imr = 0;

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

	pic_set_imr(PIC_MASTER_DT, pic_imr);
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

