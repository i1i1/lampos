#include "kernel.h"
#include "alloc.h"
#include "physpgalloc.h"

/* First fit heap allocator */


struct alloc_area_t *heap_beg;
struct alloc_area_t *heap_end;


void *
kalloc(size_t size)
{
	struct alloc_area_t *cur;

	for (cur = heap_beg; cur != heap_end; cur = NEXT_AREA(cur)){
		if (cur->flag)
			continue;

		if (cur->size - sizeof(struct alloc_area_t) * 2 < size)
			continue;

		cur->size -= size + sizeof(struct alloc_area_t);

		cur = NEXT_AREA(cur);

		cur->size = size + sizeof(struct alloc_area_t);
		cur->flag = 1;

		return cur + 1;
	}

	return NULL;
}

void
kfree(void *ptr)
{
	struct alloc_area_t *prev, *cur;

	prev = heap_beg;

	if (prev + 1 == ptr) {
		prev->flag = 0;
		return;
	}

	for (cur = NEXT_AREA(prev); cur != heap_end; prev = cur,
						cur = NEXT_AREA(cur)) {
		if (cur + 1 != ptr)
			continue;

		if (prev->flag == 0) {
			prev->size += cur->size;
			cur = prev;
		}

		cur->flag = 0;
		prev = cur;
		cur = NEXT_AREA(prev);

		if (cur != heap_end && cur->flag == 0)
			prev->size += cur->size;

		return;
	}
	iprintf("in free\n");
}

void
kalloc_info()
{
	size_t used;
	struct alloc_area_t *i;

	used = 0;

	for (i = heap_beg; i != heap_end; i = NEXT_AREA(i))
		if (i->flag)
			used += i->size;

	iprintf("\nKERNEL ALLOCATOR INFO:\n");
	iprintf("\ttotal:\t0x%x bytes\n", heap_end - heap_beg);
	iprintf("\tfree:\t0x%x bytes\n", heap_end - heap_beg - used);
	iprintf("\tused:\t0x%x bytes\n", used);
	iprintf("\theap begins at\t0x%08x\n", heap_beg);
	iprintf("\theap ends at  \t0x%08x\n", heap_end);
}

void
kalloc_init()
{
	heap_beg = (void *)physpgalloc();
	heap_end = heap_beg + 0x1000;

	heap_beg->size = (size_t)heap_end - (size_t)heap_beg;
	heap_beg->flag = 0;
}

