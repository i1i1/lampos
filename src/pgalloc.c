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
	if (--pgref[pgflags(virt) >> 12] == 0)
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
pgmalloc()
{
	size_t i, pg;

	for (i = KERNEL_BASE; i; i += 0x1000) {
		if (i == PGTEMPO || (pgflags((void *)i) & PG_PRESENT))
			continue;

		if (!(pg = physpgmalloc()))
			return NULL;

		pgmap(pg, (void *)i, PG_PRESENT|PG_RW);

		return (void *)i;
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
	size_t mapped, unmapped;
	size_t start, flags, *pgaddr;
	int i, j;

	mapped = unmapped = 0;
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
				mapped += 0x1000;
			else
				unmapped += 0x1000;
		}
	}

	tempounmap();

	iprintf("\t[0x%08x; 0x%08x] flags=%03x\n", start, 0xffffffff, flags);

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

	iprintf("\n\tmap:\t");
	if (mapped >> 30)
		iprintf("%d Gb\t", mapped >> 30);
	if ((mapped >> 20) % 1024)
		iprintf("%d Mb\t", (mapped >> 20) % 1024);
	if ((mapped >> 10) % 1024)
		iprintf("%d Kb\t", (mapped >> 10) % 1024);
	if (mapped % 1024)
		iprintf("%d b", mapped % 1024);

	iprintf("\n\ttotal:\t%d Gb\n", 4);
	}
}


/* Hack:
 *
 * Returnes one free page(4 Kb) for buddy allocator
 */
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

	return (void *)kerend;
}

void
mem_init(struct mm_area **mmap, int mmap_len)
{
	size_t kerend, i;
	void *pg;

	assert_or_panic(mmap, "Invalid pointer to memory map structure");

	kerend = (size_t)&end;

	if (kerend % 0x1000)
		kerend += 0x1000 - kerend % 0x1000;

	dprintf("\tkerend = %p\n", kerend);

	pg = pginit(kerend);
	balloc_init(pg);
	physpginit(mmap, mmap_len);

	/* Allocating 128 Kb for buddyallocator */
	for (i = 0; i < 128 * 1024; i += 0x1000) {
		pg = pgmalloc();

		if (!pg)
			goto end;

		buddyaddmem(pg, 12);
	}

end:
	iprintf("\nMemory setted up\n");
	return;
}

