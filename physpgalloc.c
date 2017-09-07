#include "defs.h"
#include "kernel.h"
#include "mb_parce.h"

#include "physpgalloc.h"


#define PGDIR_PRESENT		1
#define PGDIR_RW		(1 << 1)
#define PGDIR_USER		(1 << 2)
#define PGDIR_ACCESS		(1 << 5)
#define PGDIR_DIRTY		(1 << 6)
#define PGDIR_ALLOCATED		(1 << 9)


size_t pagedir[1024] __attribute__ ((aligned(4096))) = {0};
size_t pagetables[1024][1024] __attribute__ ((aligned(4096))) = {{0}};
int diridx;


extern uint32_t endkernel;
extern void en_pg(size_t *);


physaddr_t
physpgalloc()
{
	int i, j;
	size_t *pageaddr;

	for (i = 0; i < 1024; i++) {
		if (!(pagedir[i] & PGDIR_PRESENT))
			continue;

		pageaddr = (size_t *)(pagedir[i] & 0xfffff000);

		for (j = 0; j < 1024; j++) {
			if (pageaddr[j] & PGDIR_ALLOCATED ||
					!(pageaddr[j] & PGDIR_PRESENT))
				continue;

			pageaddr[j] |= PGDIR_ALLOCATED;
			return pageaddr[j] & 0xfffff000;
		}
	}

	return 0;
}

void
physpgfree(physaddr_t page)
{
	size_t *pageaddr;

	if (pagedir[page >> 22] & PGDIR_PRESENT) {
		pageaddr = (size_t *)(pagedir[page >> 22] & 0xfffff000);

		pageaddr[(page >> 12) % 1024] &= ~PGDIR_ALLOCATED;
	}
}

void
pagemap(physaddr_t phys, vaddr_t virt, size_t flags)
{
	size_t *pageaddr;

	if (!(pagedir[virt >> 22] & PGDIR_PRESENT))
		pagedir[virt >> 22] = (size_t)pagetables[diridx++] | flags;

	pageaddr = (size_t *)(pagedir[virt >> 22] & 0xfffff000);
	pageaddr[(virt >> 12) % 1024] = phys | flags;
}

void
physpginit(struct area **buf, int buflen)
{
	size_t i;

	diridx = 0;

	for (i = 0; i < 0x1000000; i += 0x1000)
		pagemap(i, i, PGDIR_PRESENT + PGDIR_RW + PGDIR_ALLOCATED);

	for (; i < (buf[buflen - 1]->end & 0xfffff000); i += 0x1000)
		pagemap(i, i, PGDIR_PRESENT + PGDIR_RW);

	en_pg(pagedir);

	iprintf("\tHELLO PAGING WORLD\n");
}

