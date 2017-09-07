#ifndef _KALLOC_H_
#define _KALLOC_H_

#include "defs.h"


#define NEXT_ALLOC_AREA(a)	((struct alloc_area *)((size_t)(a) + (a)->size))


struct alloc_area {
	uint16_t size	: 15;
	uint8_t flag	: 1;
} __attribute__((packed));


void *kalloc(size_t size);
void kfree(void *ptr);
void kalloc_info();
void kalloc_init();


#endif /* _KALLOC_H_ */

