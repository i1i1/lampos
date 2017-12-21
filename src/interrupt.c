#include "kernel.h"
#include "cpu.h"
#include "interrupt.h"

struct idt_gate intr[256] = { { 0 } };
struct idt	idt;

extern char end;

extern void load_idt(void *idt);
extern void int0_asm_handler();
extern void int14_asm_handler();
extern void idt_init();

void
int_add(int code, uint16_t segm, uint8_t type, int dpl, void *handler)
{
	intr[code].present = 1;
	intr[code].res = 0;

	intr[code].segm = sizeof(struct gdt_entry) * segm;
	intr[code].type = type;
	intr[code].dpl = dpl;
	intr[code].offset_low = (size_t)handler % (1 << 16);
	intr[code].offset_high = (size_t)handler >> 16;
}

void
zerodivision(size_t cr2)
{
	iprintf("\nZero divizion\n", cr2);
	iprintf("\n\tcr2 = %p\n", cr2);
	iprintf("KERNEL STOP\n");
	for (;;);
}

void
int_init()
{
	idt.limit = 256 * 8 - 1;
	idt.intr = intr;

	int_add( 0, 1, INT_GATE	, 0, int0_asm_handler);
	int_add(14, 1, TRAP_GATE, 0, int14_asm_handler);

	load_idt(&idt);
}

