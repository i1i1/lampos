#include <stdint.h>

#include "kernel.h"
#include "pic.h"
#include "interrupt.h"

#include "pit.h"

//https://wiki.osdev.org/Programmable_Interval_Timer

extern void pit_asm_handler();

enum pit_channel {
	PIT_CHAN0 = 0x40,
	PIT_CHAN1 = 0x41,
	PIT_CHAN2 = 0x42,
	PIT_CHAN_CMD = 0x43,
};

enum pit_cmd {
	CHAN_0 		= 0,
	CHAN_1 		= 0x40,
	CHAN_2 		= 0x80,
	READ_BACK 	= 0xC0,

	LATCH_CNT 	= 0,
	ACC_LOBYTE 	= 0x10,
	ACC_HIBYTE 	= 0x20,
	ACC_BOTH 	= 0x30,

	OP_SW_ONESHOT 	= 0,		//Interrupt On Terminal Count
	OP_HW_ONESHOT 	= 0x02,		//Not usable for Channels 0, 1
	OP_RATEGEN 	= 0x04,		//
	OP_SQUARE_WAVE 	= 0x06,
	OP_SW_STROBE 	= 0x08,
	OP_HW_STROBE 	= 0x09,

	MODE_BCD 	= 0x01,
	MODE_BINARY 	= 0,	//16-bit binary
};

int cnt = 0;
struct timer {
	uint32_t sec;
	uint32_t ms;
} g_timer;

// set freq between ~19hz and ~600khz
void
pit_set_freq(int hz)
{
	int freq = 1193182;
	int divisor = freq / hz;
	divisor = divisor & (~0x1);

	if (!divisor)	//to small
		divisor = 2;

	divisor &= 0xffff;

	iprintf("divisor = %x\n", divisor);
	outb(PIT_CHAN_CMD, CHAN_0 | ACC_BOTH | OP_RATEGEN | MODE_BINARY);
	outb(PIT_CHAN0, divisor & 0xff);
	outb(PIT_CHAN0, divisor >> 8);
}

void
pit_init()
{
	g_timer.sec = 0;
	g_timer.ms = 0;

	pit_set_freq(1000); // raise IRQ approximately every 1 ms

	int_add(IRQ_TIMER, 1, TRAP_GATE, 0, pit_asm_handler);
	pic_imr_add(IRQ_TIMER_MASK);
}

static inline void
upd_timer()
{
	g_timer.ms++;
	if (g_timer.ms >= 1000) {
		g_timer.sec++;
		g_timer.ms = 0;
	}
}

void
pit_irq()
{
	cnt++;
	upd_timer();
	pic_eoi(PIC_MASTER_CMD);
}

void
pit_sleep(int ms)
{
	struct timer tmp = g_timer;

	tmp.ms += ms % 1000;
	tmp.sec += ms / 1000 + tmp.ms / 1000;
	tmp.ms %= 1000;

	while (tmp.sec > g_timer.sec ||
	       (tmp.sec == g_timer.sec && tmp.ms > g_timer.ms)) {
		halt();
	}
}

/* Hack! */
/* Return current tick */
size_t
pit_get_ticks()
{
	return g_timer.sec * 1000 + g_timer.ms;
}

