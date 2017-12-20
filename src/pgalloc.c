#include "defs.h"
#include "kernel.h"
#include "mb_parce.h"
#include "interrupt.h"

#include "pgalloc.h"


size_t *pgdir;
const size_t *tempopage;
size_t kerend;


extern char end;
extern void pgreset();

void *
tempomap(size_t addr)
{
	size_t *pgaddr;

	pgaddr = (void *)(PGDIR + 0x2000);
	pgaddr[((size_t)tempopage >> 12) % 1024] =
		addr & ((~0xfff)|PG_PRESENT|PG_RW|PG_ALLOCATED);

	return (void *)tempopage;
}

void
pgfault(size_t cr2, size_t error)
{
	iprintf("\nPage Fault:\n\n");
	iprintf("\terror = 0x%x; cr2 = %p\n\n", error, cr2);
	iprintf("Stopping Kernel\n\n", error, cr2);

	for (;;);
}

void *
pgalloc()
{
	size_t i, j;
	size_t *pgaddr;

	for (i = 0; i < 1024; i++) {
		if (!(pgdir[i] & PG_PRESENT))
			continue;

		pgaddr = (size_t *)(pgdir[i] & (~0xfff));

		for (j = 0; j < 1024; j++)
			if (pgaddr[j] & PG_PRESENT &&
					!(pgaddr[j] & PG_ALLOCATED)) {
				pgaddr[j] |= PG_ALLOCATED;
				return (void *)((i << 22) + (j << 12));
			}
	}

	return 0;
}

size_t
virttophys(void *pg)
{
	size_t *pgaddr;

	if (pgdir[(size_t)pg >> 22] & PG_PRESENT) {
		pgaddr = (size_t *)(pgdir[(size_t)pg >> 22] & (~0xfff));

		return pgaddr[((size_t)pg >> 12) % 1024] & (~0xfff);
	}

	return 0;
}

void
pgfree(void *pg)
{
	size_t *pgaddr;

	if (pgdir[(size_t)pg >> 22] & PG_PRESENT) {
		pgaddr = (size_t *)(pgdir[(size_t)pg >> 22] & ~(0xfff));

		pgaddr[((size_t)pg >> 12) % 1024] &= ~PG_ALLOCATED;
	}
}

void
pginfo()
{
	size_t free, used, unmapped, total;
	size_t start, flags, *pgaddr;
	int i, j;

	free = used = unmapped = 0;
	start = 0;

	if (pgdir[0] & PG_PRESENT) {
		pgaddr = tempomap(pgdir[0]);
		flags = pgaddr[0] & 0xf9f;
	}
	else
		flags = 0;

	iprintf("\n\nPAGE ALLOCATOR INFO:\n");

	for (i = 0; i < 1024; i++) {
		if (!(pgdir[i] & PG_PRESENT)) {
			unmapped += 0x400000;
			if (flags) {
				iprintf("\t[0x%08x; 0x%08x) flags=%03x\n",
						start, i << 22, flags);
				start = i << 22;
				flags = 0;
			}
			continue;
		}

		pgaddr = tempomap(pgdir[i]);

		for (j = 0; j < 1024; j++) {
			if (flags != (pgaddr[j] & 0xf9f)) {
				iprintf("\t[0x%08x; 0x%08x) flags=%03x\n",
						start, ((i << 10) + j) << 12, flags);
				flags = pgaddr[j] & 0xf9f;
				start = ((i << 10) + j) << 12;
			}
			if (pgaddr[j] & PG_PRESENT)
				if (pgaddr[j] & PG_ALLOCATED)
					used += 0x1000;
				else
					free += 0x1000;
			else
				unmapped += 0x1000;
		}
	}

	iprintf("\t[0x%08x; 0x%08x] flags=%03x\n", start, 0xffffffff, flags);

	total = used + free;
	{
	iprintf("\n\tunmap:\t");
	if (unmapped >> 30)
		iprintf("%d Gb\t", unmapped >> 30);
	if ((unmapped >> 20) % 1024)
		iprintf("%d Mb\t", (unmapped >> 20) % 1024);
	if ((unmapped >> 10) % 1024)
		iprintf("%d Kb\t", (unmapped >> 10) % 1024);
	if (unmapped % 1024)
		iprintf("%d b", unmapped % 1024);
	iprintf("\n\n");

	iprintf("\ttotal:\t");
	if (total >> 30)
		iprintf("%d Gb\t", total >> 30);
	if ((total >> 20) % 1024)
		iprintf("%d Mb\t", (total >> 20) % 1024);
	if ((total >> 10) % 1024)
		iprintf("%d Kb\t", (total >> 10) % 1024);
	if (total % 1024)
		iprintf("%d b", total % 1024);
	iprintf("\n");

	iprintf("\tfree:\t");
	if (free >> 30)
		iprintf("%d Gb\t", free >> 30);
	if ((free >> 20) % 1024)
		iprintf("%d Mb\t", (free >> 20) % 1024);
	if ((free >> 10) % 1024)
		iprintf("%d Kb\t", (free >> 10) % 1024);
	if (free % 1024)
		iprintf("%d b", free % 1024);
	iprintf("\n");

	iprintf("\tused:\t");
	if (used >> 30)
		iprintf("%d Gb\t", used >> 30);
	if ((used >> 20) % 1024)
		iprintf("%d Mb\t", (used >> 20) % 1024);
	if ((used >> 10) % 1024)
		iprintf("%d Kb\t", (used >> 10) % 1024);
	if (used % 1024)
		iprintf("%d b", used % 1024);
	iprintf("\n");
	}
}

void
pgtablemap(size_t phys, size_t virt, size_t flags)
{
	pgdir[virt >> 22] = phys | flags;
}

void
pgmap(size_t phys, size_t virt, size_t flags)
{
	size_t *pgaddr;

	if (!(pgdir[virt >> 22] & PG_PRESENT))
		return;

	pgaddr = tempomap(pgdir[virt >> 22]);

	pgaddr[(virt >> 12) % 1024] = phys | flags;
}

void
pginit(struct mm_area **mmap, int mmap_len)
{
	size_t i;
	pgdir = (void *)PGDIR;

	kerend = (size_t)&end;
	kerend += (kerend % 0x1000 ? 0x1000 : 0) - kerend % 0x1000;
	tempopage = (size_t *)(PGDIR + 0x3000);

	iprintf("\tkerend = %p\n", kerend);

	pginfo();

	pgtablemap(0, 0, 0);
	pgtablemap(0, 1 << 22, 0);

	for (i = KERNEL_BASE; i <= kerend; i += 0x1000)
		pgmap(i - KERNEL_BASE, i, PG_PRESENT|PG_RW|PG_ALLOCATED);

	for (; i < PGDIR; i += 0x1000)
		pgmap(0, i, 0);

	for (; i <= PGDIR + 0x3000; i += 0x1000)
		pgmap(i - KERNEL_BASE, i, PG_PRESENT|PG_RW|PG_ALLOCATED);

	pginfo();
}

