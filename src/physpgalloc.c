#include "defs.h"
#include "kernel.h"

#include "buddyalloc.h"
#include "pgalloc.h"
#include "physpgalloc.h"


struct physarea_lst {
	struct physarea_lst *next;
	paddr_t beg;
	paddr_t end;

} *head = NULL;

size_t physpgtotal = 0;

paddr_t
physpgmalloc()
{
	size_t pg;
	void *tmp;

	if (!head)
		return 0;

	pg = head->beg;
	head->beg += 0x1000;

	if (head->beg == head->end) {
		tmp = head;
		head = head->next;

		bfree(tmp);
	}

	return pg;
}

void
physpgfree(paddr_t phys)
{
	struct physarea_lst *np, *tmp;

	/* If no chunks just addes one */
	if (!head) {
		head = balloc(sizeof(struct physarea_lst));

		if (!head)
			return;

		head->beg = phys;
		head->end = phys + 0x1000;
		head->next = NULL;

		return;
	}

	/* Tries to find 2 neighbor chunks. Phys is betwen them  */
	for (np = head; np->next; np = np->next)
		if (np->end <= phys && phys < np->next->beg)
			break;

	/* If can resize left chunk, just resizes it */
	if (np->end == phys) {
		np->end += 0x1000;
		return;
	}

	/* If can resize right chunk, just resizes it */
	if (np->next && np->next->beg - 0x1000 == phys) {
		np->next->beg -= 0x1000;
		return;
	}

	/* Else addes one more free chunk */
	tmp = balloc(sizeof(struct physarea_lst));

	if (!tmp)
		return;

	tmp->beg = phys;
	tmp->end = phys + 0x1000;
	tmp->next = np->next;

	np->next = tmp;
}

void
physpgadd(paddr_t p)
{
	physpgfree(p);
}

void
physpginfo()
{
	size_t used, free, total;
	struct physarea_lst *np;

	free = 0;
	total = physpgtotal;

	iprintf("\n\nPHYSPAGEALLOC INFO:\n");

	for (np = head; np; np = np->next) {
		dprintf("\t[0x%08x to 0x%08x)\n", np->beg, np->end);
		free += np->end - np->beg;
	}

	used = total - free;

	{

	iprintf("\n\ttotal:\t");
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

int
in_pg(size_t phys)
{
	size_t pg;
	vaddr_t tmp;

	tmp.num = phys + KERNEL_BASE;

	pg = pgflags(tmp).num;

	if (pg & PG_PRESENT) {
		pg &= ~0xfff;

		if (pg == phys)
			return 0;
	}

	return 1;
}

int
in_mm_area(size_t phys, struct mm_area **mmap, int mmap_len)
{
	int i;

	for (i = 0; i < mmap_len; i++)
		if (mmap[i]->beg <= phys && phys < mmap[i]->end)
			return 1;

	return 0;
}

void
physpginit(struct mm_area **mmap, int mmap_len)
{
	size_t i;

	physpgtotal = mmap[mmap_len - 1]->end;

	for (i = 0x0; i < physpgtotal; i += 0x1000)
		if (in_pg(i) && in_mm_area(i, mmap, mmap_len))
			physpgadd(i);
}

