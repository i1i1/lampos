#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "defs.h"

#define	TASK_GATE	0x5
#define	INT_GATE	0xE
#define	TRAP_GATE	0xF


struct idt_gate {
	uint16_t offset_low	:16;
	uint16_t segm		:16;
	uint16_t res		:8;
	uint16_t type		:5;
	uint16_t dpl		:2;
	uint16_t present	:1;
	uint16_t offset_high	:16;
} __attribute__ ((packed));

struct idt {
	uint16_t	limit;
	struct idt_gate *intr;
} __attribute__ ((packed));

void int_add(int code, uint16_t segm, uint8_t type, int dpl, void *handler);

void int_init();

#endif /* _INTERRUPT_H_ */

