#include "defs.h"
#include "kernel.h"
#include "mb_parce.h"
#include "interrupt.h"

#include "buddyalloc.h"
#include "pgalloc.h"
#include "physpgalloc.h"


#define	pgdirunmap(virt)	pgdirmap(0, virt, 0)

extern char end;

size_t *pgdir;
const size_t *tempopage;
uint8_t pgref[1024 * 1024] = { 0 };


inline void
pgreset()
{
	__asm__ __volatile__ ("mov %cr3, %eax; mov %eax, %cr3");
}

void *
tempomap(size_t addr)
{
	size_t *pgaddr;

	pgaddr = (void *)PGTBL1;
	pgaddr[((size_t)tempopage << 10) >> 22] =
		addr & ((~0xfff)|PG_PRESENT|PG_RW|PG_ALLOCATED);
	pgreset();

	return (void *)tempopage;
}

void
tempounmap()
{
	size_t *pgaddr;

	pgaddr = (void *)PGTBL1;
	pgaddr[((size_t)tempopage << 10) >> 22] = 0;

	pgreset();
}

size_t
pgdirflags(void *virt)
{
	return *(pgdir + ((size_t)virt >> 22));
}

size_t
pgflags(void *virt)
{
	size_t *pgaddr;

	if (!(pgdirflags(virt) & PG_PRESENT) || virt == (void *)PGTEMPO)
		return 0;

	pgaddr = tempomap(pgdir[(size_t)virt >> 22]);

	return pgaddr[((size_t)virt << 10) >> 22];
}

void
pgdirmap(size_t phys, void *virt, size_t flags)
{
	pgdir[(size_t)virt >> 22] = phys | flags;
}

void
pgmap_force(size_t phys, void *virt, size_t flags)
{
	size_t *pgaddr;

	pgaddr = tempomap(pgdir[(size_t)virt >> 22]);
	pgaddr[((size_t)virt << 10) >> 22] = phys | flags;

	tempounmap();
}

void
pgmap(size_t phys, void *virt, size_t flags)
{
	size_t pgaddr;

	if (pgdirflags(virt) & PG_PRESENT)
		goto pgmap_exit;

	pgaddr = physpgmalloc();

	if (!pgaddr)
		return;

	pgdirmap(pgaddr, virt, flags);

pgmap_exit:
	pgmap_force(phys, virt, flags);
	pgref[phys >> 12]++;
}

void
pgunmap(void *virt)
{
	if (--pgref[pgflags(virt) >> 12])
		physpgfree(pgflags(virt));

	pgmap_force(0, virt, 0);
}

void
pgfault(size_t error, size_t addr)
{
	iprintf("\nPage Fault:\n\n");
	iprintf("\taddr = 0x%x; error = %p\n\n", addr, error);
	iprintf("Stopping Kernel\n\n");

	for (;;);
}

void *
pgalloc()
{
	size_t i, pg;

	for (i = 0x1000; i != 0; i += 0x1000) {
		if (i != PGTEMPO && !(pgflags(i) & PG_PRESENT)) {
			if (!(pgdirflags(i) & PG_PRESENT)) {
				if (!(pg = physpgmalloc()))
					return NULL;

				pgdirmap(i, pg, PG_PRESENT|PG_RW);
			}

			if (!(pg = physpgmalloc()))
				return NULL;

			pgmap(i, pg, PG_PRESENT|PG_RW);

			return i;
		}
	}

	return NULL;
}

void
pgfree(void *virt)
{
	pgunmap(virt);
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

	tempounmap();

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


/* Returnes one free page for buddy allocator */
void *
pginit(size_t kerend)
{
	size_t i;

	pgdir = (void *)PGDIR;
	tempopage = (size_t *)PGTEMPO;

	for (i = KERNEL_BASE; i <= kerend; i += 0x1000)
		pgmap(i - KERNEL_BASE, (void *)i, PG_PRESENT|PG_RW);

	for (; i < PGDIR; i += 0x1000)
		pgmap_force(0, (void *)i, 0);

	for (; i < PGTEMPO; i += 0x1000)
		pgmap(i - KERNEL_BASE, (void *)i, PG_PRESENT|PG_RW);

	for (i += 0x1000; i < KERNEL_BASE + (2 << 22); i += 0x1000)
		pgmap_force(0, (void *)i, 0);

	pginfo();

	return (void *)kerend;
}

void
kmeminit(struct mm_area **mmap, int mmap_len)
{
	size_t kerend;
	void *pg;

	kerend = (size_t)&end;

	if (kerend % 0x1000)
		kerend += 0x1000 - kerend % 0x1000;

	dprintf("\tkerend = %p\n", kerend);

	pg = pginit(kerend);

	balloc_init(pg);

	physpginit(mmap, mmap_len);
}

