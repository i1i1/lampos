#include "defs.h"
#include "kernel.h"
#include "mb_parce.h"
#include "interrupt.h"

#include "pgalloc.h"


size_t pgdir[1024] __attribute__ ((aligned(4096))) = {0};
size_t pgtables[1024][1024] __attribute__ ((aligned(4096))) = {{0}};
int nextdir;
size_t kerend;


extern char end;
extern void pgenable(size_t *);
extern void int14_asm_handler();

void
pgfault(size_t cr2, size_t error)
{
	if (!(error & 0))
		pgmap(cr2, cr2, PGDIR_PRESENT|PGDIR_ALLOCATED|PGDIR_RW);

//	iprintf("\n\tPage Fault\n");
//	iprintf("\terror = 0x%x; cr2 = %p\n\n", error, cr2);
}

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
	size_t start, flags;
	int i, j;

	free = used = 0;
	start = 0;
	flags = 0x203;

	iprintf("\n\nPAGE ALLOCATOR INFO:\n");

	for (i = 0; i < 1024; i++) {
		if (!(pgdir[i] & PGDIR_PRESENT))
			continue;

		pgaddr = (size_t *)(pgdir[i] & 0xfffff000);

		for (j = 0; j < 1024; j++) {
			if (flags != (pgaddr[j] & 0xf9f)) {
				if (flags & PGDIR_PRESENT && ((i << 10) + j) << 12)
					iprintf("\tarea from 0x%08x to 0x%08x with flags %03x\n", start, ((i << 10) + j) << 12, flags);
				flags = pgaddr[j] & 0xf9f;
				start = ((i << 10) + j) << 12;
			}
			if (pgaddr[j] & PGDIR_PRESENT) {
				if (pgaddr[j] & PGDIR_ALLOCATED)
					used += 0x1000;
				else
					free += 0x1000;
			}
		}
	}

	if (flags & PGDIR_PRESENT && ((i << 10) + j) << 12)
		iprintf("\tarea from 0x%08x to 0x%08x with flags %03x\n",
				start, ((i << 10) + j) << 12, flags);

	total = used + free;

	iprintf("\n\ttotal:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n", total >> 30,\
		(total >> 20) % 1024, (total >> 10) % 1024, total % 1024);
	iprintf("\tfree:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n", free >> 30,\
			(free >> 20) % 1024, (free >> 10) % 1024, free % 1024);
	iprintf("\tused:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n\n", used >> 30,\
			(used >> 20) % 1024, (used >> 10) % 1024, used % 1024);
}

int a = 0;

void
pgmap(size_t phys, size_t virt, size_t flags)
{
	register size_t *pgaddr;

	if (!(pgdir[virt >> 22] & PGDIR_PRESENT))
		pgdir[virt >> 22] = ((size_t)pgtables[nextdir++] &
				0xfffff000) | flags;

	pgaddr = (size_t *)(pgdir[virt >> 22] & 0xfffff000);
	pgaddr[(virt >> 12) % 1024] = phys | flags;
}

void
pginit(struct mm_area **mmap, int mmap_len)
{
	size_t i, j;

	kerend = (size_t)&end;
	kerend += (kerend % 0x1000 ? 0x1000 : 0) - kerend % 0x1000;
	nextdir = 0;

	iprintf("\tkerend = %p\n", kerend);

	/* Mapping memory from mmap */
	for (i = 0; (int)i < mmap_len; i++)
		for (j = mmap[i]->beg; j < (mmap[i]->end & 0xfffff000); j += 0x1000)
			pgmap(j, j, PGDIR_PRESENT|PGDIR_RW);

	/* Making kernel not allocatable */
	for (i = 0; i <= kerend; i += 0x1000)
		pgmap(i, i, PGDIR_PRESENT|PGDIR_ALLOCATED|PGDIR_RW);

	pgmap((size_t)pgdir, (size_t)pgdir, PGDIR_PRESENT|PGDIR_ALLOCATED|PGDIR_RW);

	for (i = 0; i < 1024; i++)
		pgmap((size_t)pgtables[i], (size_t)pgtables[i], PGDIR_PRESENT|PGDIR_ALLOCATED|PGDIR_RW);

	int_add(14, 1, TRAP_GATE, 0, int14_asm_handler);

//	pginfo();

	pgenable(pgdir);

	iprintf("\tHELLO PAGING WORLD\n");
}

