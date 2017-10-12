#include "defs.h"
#include "kernel.h"
#include "mb_parce.h"

#include "pgalloc.h"


#define PGDIR_PRESENT		1
#define PGDIR_RW		(1 << 1)
#define PGDIR_USER		(1 << 2)
#define PGDIR_ACCESS		(1 << 5)
#define PGDIR_DIRTY		(1 << 6)
#define PGDIR_ALLOCATED		(1 << 9)


size_t pgdir[1024] __attribute__ ((aligned(4096))) = {0};
size_t pgtables[1024][1024] __attribute__ ((aligned(4096))) = {{0}};
int nextdir;


extern char end;
extern void pgenable(size_t *);


void *
pgalloc()
{
	size_t i, j;
	size_t *pgaddr;

	for (i = 0; i < 1024; i++) {
		if (!(pgdir[i] & PGDIR_PRESENT))
			continue;

		pgaddr = (size_t *)(pgdir[i] & 0xfffff000);

		for (j = 0; j < 1024; j++)
			if (pgaddr[j] & PGDIR_PRESENT &&
					!(pgaddr[j] & PGDIR_ALLOCATED)) {
				pgaddr[j] |= PGDIR_ALLOCATED;
				return (void *)((i << 22) + (j << 12));
			}
	}

	return 0;
}

size_t
virttophys(void *pg)
{
	size_t *pgaddr;

	if (pgdir[(size_t)pg >> 22] & PGDIR_PRESENT) {
		pgaddr = (size_t *)(pgdir[(size_t)pg >> 22] & 0xfffff000);

		return pgaddr[((size_t)pg >> 12) % 1024] & 0xfffff000;
	}

	return 0;
}

void
pgfree(void *pg)
{
	size_t *pgaddr;

	if (pgdir[(size_t)pg >> 22] & PGDIR_PRESENT) {
		pgaddr = (size_t *)(pgdir[(size_t)pg >> 22] & 0xfffff000);

		pgaddr[((size_t)pg >> 12) % 1024] &= ~PGDIR_ALLOCATED;
	}
}

void
pginfo()
{
	size_t free, used, total;
	size_t *pgaddr;
	int i, j;

	free = used = 0;

	for (i = 0; i < 1024; i++) {
		if (!(pgdir[i] & PGDIR_PRESENT))
			continue;

		pgaddr = (size_t *)(pgdir[i] & 0xfffff000);

		for (j = 0; j < 1024; j++) {
			if (pgaddr[j] & PGDIR_PRESENT) {
				if (pgaddr[j] & PGDIR_ALLOCATED)
					used += 0x1000;
				else
					free += 0x1000;
			}
		}
	}

	total = used + free;

	iprintf("\n\nPAGE ALLOCATOR INFO:\n");
	iprintf("\ttotal:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n", total >> 30,\
		(total >> 20) % 1024, (total >> 10) % 1024, total % 1024);
	iprintf("\tfree:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n", free >> 30,\
			(free >> 20) % 1024, (free >> 10) % 1024, free % 1024);
	iprintf("\tused:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n\n", used >> 30,\
			(used >> 20) % 1024, (used >> 10) % 1024, used % 1024);
}

void
pgmap(size_t phys, void *virt, size_t flags)
{
	size_t *pgaddr;

	if (!(pgdir[(size_t)virt >> 22] & PGDIR_PRESENT))
		pgdir[(size_t)virt >> 22] = (size_t)pgtables[nextdir++] | flags;

	pgaddr = (size_t *)(pgdir[(size_t)virt >> 22] & 0xfffff000);
	pgaddr[((size_t)virt >> 12) % 1024] = phys | flags;
}

void
pginit(struct mm_area **mmap, int mmap_len)
{
	size_t i, j;

	nextdir = 0;

	/* Mapping memory from mmap */
	for (i = 0; (int)i < mmap_len; i++)
		for (j = mmap[i]->beg; j < (mmap[i]->end & 0xfffff000); j += 0x1000)
			pgmap(j, (void *)j, PGDIR_PRESENT + PGDIR_RW);

	/* Mapping first 16 Mb as if they were allocated */
	for (i = 0; i < 0x1000000; i += 0x1000)
		pgmap(i, (void *)i, PGDIR_PRESENT + PGDIR_RW + PGDIR_ALLOCATED);

	pgenable(pgdir);

	iprintf("\tHELLO PAGING WORLD\n");
}

