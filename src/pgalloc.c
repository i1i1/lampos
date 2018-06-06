#include "defs.h"
#include "kernel.h"
#include "mb_parce.h"
#include "interrupt.h"

#include "buddyalloc.h"
#include "pgalloc.h"
#include "physpgalloc.h"


#define	pgdirunmap(virt)	pgdirmap(0, virt, 0)

extern char end;

vaddr_t *pgdir = (vaddr_t *)PGDIR;
const vaddr_t tempopage = { .ptr = (void *)PGTEMPO, };
uint8_t pgref[1024 * 1024] = { 0 };

inline void
pgreset()
{
	__asm__ __volatile__ ("mov %cr3, %eax; mov %eax, %cr3");
}

vaddr_t
tempomap(vaddr_t addr)
{
	vaddr_t *pgaddr;

	pgaddr = (void *)PGTBL1;

	pgaddr[tempopage.tbl.ent] = addr;
	pgaddr[tempopage.tbl.ent].tbl.off = PG_PRESENT|PG_RW|PG_ALLOCATED;

	pgreset();

	return tempopage;
}

void
tempounmap()
{
	size_t *pgaddr;

	pgaddr = (void *)PGTBL1;
	pgaddr[tempopage.tbl.ent] = 0;

	pgreset();
}

/* TODO: Find a better name! */
/* Eventually returns whole structure */
vaddr_t
pgdirflags(vaddr_t v)
{
	return pgdir[v.tbl.dir];
}

/* TODO: Find a better name! */
/* Eventually returns whole structure */
vaddr_t
pgflags(vaddr_t v)
{
	vaddr_t *pgaddr, ret;

	if (!(pgdirflags(v).tbl.off & PG_PRESENT) ||
	    v.ptr == tempopage.ptr) {
		ret.num = 0;
		return ret;
	}

	pgaddr = tempomap(pgdir[v.tbl.dir]).ptr;

	return pgaddr[v.tbl.ent];
}

void
pgdirmap(paddr_t p, vaddr_t v, uint16_t flags)
{
	pgdir[v.tbl.dir].num = p | flags;
}

void
pgmap_force(paddr_t p, vaddr_t v, size_t flags)
{
	size_t *pgaddr;

	pgaddr = tempomap(pgdir[v.tbl.dir]).ptr;
	pgaddr[v.tbl.ent] = p | flags;

	tempounmap();
}

void
pgmap(paddr_t p, vaddr_t v, uint16_t flags)
{
	size_t pgaddr;

	if (pgdirflags(v).tbl.off & PG_PRESENT)
		goto pgmap_exit;

	pgaddr = physpgmalloc();

	if (!pgaddr)
		return;

	pgdirmap(pgaddr, v, flags);

pgmap_exit:
	pgmap_force(p, v, flags);
	pgref[p >> 12]++;
}

void
pgunmap(vaddr_t v)
{
	if (--pgref[pgflags(v).num >> 12] == 0)
		physpgfree(pgflags(v).num);

	pgmap_force(0, v, 0);
}

void
pgfault(size_t error, size_t addr)
{
	iprintf("\nPage Fault:\n\n");
	iprintf("\taddr = 0x%x; error = %p\n\n", addr, error);
	iprintf("Stopping Kernel\n\n");

	for (;;);
}

vaddr_t
pgmalloc()
{
	vaddr_t i, ret;
	paddr_t p;

	for (i.num = KERNEL_BASE; i.ptr; i.num += 0x1000) {
		if (i.num == PGTEMPO ||
		    (pgflags(i).tbl.off & PG_PRESENT))
			continue;

		if (!(p = physpgmalloc())) {
			ret.ptr = NULL;
			return ret;
		}

		pgmap(p, i, PG_PRESENT|PG_RW);

		return i;
	}

	ret.ptr = NULL;
	return ret;
}

void
pgfree(vaddr_t virt)
{
	pgunmap(virt);
}

void
pginfo()
{
	size_t mapped, unmapped;
	size_t start, flags;
	vaddr_t *pgaddr;
	int i, j;

	mapped = unmapped = 0;
	start = 0;

	if (pgdir[0].num & PG_PRESENT) {
		pgaddr = tempomap(pgdir[0]).ptr;
		flags = pgaddr[0].tbl.off & 0xf9f;
	}
	else
		flags = 0;

	iprintf("\n\nPAGE ALLOCATOR INFO:\n");

	for (i = 0; i < 1024; i++) {
		if (!(pgdir[i].tbl.off & PG_PRESENT)) {
			unmapped += 0x400000;
			if (flags) {
				iprintf("\t[0x%08x; 0x%08x) flags=%03x\n",
						start, i << 22, flags);
				start = i << 22;
				flags = 0;
			}
			continue;
		}

		pgaddr = tempomap(pgdir[i]).ptr;

		for (j = 0; j < 1024; j++) {
			if (flags != (pgaddr[j].tbl.off & 0xf9f)) {
				iprintf("\t[0x%08x; 0x%08x) flags=%03x\n",
						start, ((i << 10) + j) << 12, flags);
				flags = pgaddr[j].tbl.off & 0xf9f;
				start = ((i << 10) + j) << 12;
			}
			if (pgaddr[j].tbl.off & PG_PRESENT)
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
vaddr_t
pginit(vaddr_t kerend)
{
	vaddr_t i;

	for (i.num = KERNEL_BASE; i.ptr <= kerend.ptr; i.num += 0x1000)
		pgmap(i.num - KERNEL_BASE, i, PG_PRESENT|PG_RW);

	for (; i.num < PGDIR; i.num += 0x1000)
		pgmap_force(0, i, 0);

	for (; i.num < PGTEMPO; i.num += 0x1000)
		pgmap(i.num - KERNEL_BASE, i, PG_PRESENT|PG_RW);

	for (i.num += 0x1000; i.num < KERNEL_BASE + (2 << 22); i.num += 0x1000)
		pgmap_force(0, i, 0);

	return kerend;
}

void
mem_init(struct mm_area **mmap, int mmap_len)
{
	size_t i;
	vaddr_t pg, kerend;

	assert_or_panic(mmap, "Invalid pointer to memory map structure");

	kerend.ptr = &end;
	if (kerend.tbl.off) {
		kerend.tbl.off = 0;
		kerend.tbl.ent++;

		if (kerend.tbl.ent == 0)
			kerend.tbl.dir++;
	}

	dprintf("dir = 0x%x; ent = 0x%x; off = 0x%x;\n", kerend.tbl.dir,
		kerend.tbl.ent, kerend.tbl.off);
	dprintf("num = 0x%x\n", kerend.num);

	pg = pginit(kerend);
	pginfo();
	balloc_init(pg.ptr);
	physpginit(mmap, mmap_len);

	/* Allocating 128 Kb for buddyallocator */
	for (i = 0; i < 128 * 1024; i += 0x1000) {
		pg = pgmalloc();

		if (!pg.ptr)
			goto end;

		buddyaddmem(pg.ptr, 12);
	}

end:
	iprintf("\nMemory setted up\n");
	return;
}

