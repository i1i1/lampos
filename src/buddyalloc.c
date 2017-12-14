#include "buddyalloc.h"
#include "pgalloc.h"
#include "kernel.h"


struct buddy_lst *buddies[MAXBUDDY + 1] = {NULL};


int
mergeable(struct buddy_lst *a, struct buddy_lst *b, int power)
{
	if (power == MAXBUDDY)
		return 0;

	if (a > b)
		SWAP(a, b);

	a = (void *)((size_t)a >> power);
	b = (void *)((size_t)b >> power);

	if ((size_t)a + 1 != (size_t)b)
		return 0;

	if ((size_t)a % 2 == 1 && (size_t)b % 2 == 0)
		return 0;

	return 1;
}

void
addbuddy(struct buddy_lst *addr, int power, int flag)
{
	struct buddy_lst *np;

	if (addr == NULL)
		return;

	np = buddies[power];

	/* If no elementes in list */
	if (np == NULL) {
		buddies[power] = addr;
		buddies[power]->prev = buddies[power]->next = NULL;
		buddies[power]->flag = flag;

		return;
	}

	/* If only 1 element in list */
	if (np->next == NULL) {
		if (flag != BUDDY_ALLOCATED && np->flag == BUDDY_FREE &&
				mergeable(np, addr, power)) {
			buddies[power] = NULL;
			addbuddy(MIN(np, addr), power + 1, flag);
			return;
		}

		if (np < addr) {
			np->next = addr;
			addr->next = NULL;
			addr->prev = np;
		}
		else {
			buddies[power] = addr;
			addr->next = np;
			addr->prev = NULL;
		}

		addr->flag = flag;
		return;
	}

	for (; np->next != NULL; np = np->next)
		if (np->next > addr)
			break;

	if (flag == BUDDY_ALLOCATED)
		goto addbuddy_finalize;

	/* Trying to merge with left one */
	if (np->flag == BUDDY_FREE && mergeable(np, addr, power)) {
		np->prev->next = np->next;
		addbuddy(np, power + 1, flag);
		return;
	}

	/* Trying to merge with right one */
	if (np->next && np->next->flag == BUDDY_FREE && mergeable(addr, np->next, power)) {
		np->next = np->next->next;
		addbuddy(addr, power + 1, flag);
		return;
	}

addbuddy_finalize:
	/* IF not mergeable with other areas or allocated */
	addr->next = np->next;
	addr->prev = np;
	np->next->prev = addr;
	np->next = addr;
	addr->flag = flag;
}

void *
balloc(size_t size)
{
	struct buddy_lst *np;
	int i, pow;

	/* Finding minimum power for this size */
	for (pow = 0; (1 << pow) < sizeof(struct buddy_lst); pow++)
		;

	for (; (1 << pow) - sizeof(struct buddy_lst) < size; pow++)
		;

	/* Finding fitting buddy */
	for (i = pow; i < MAXBUDDY; i++)
		for (np = buddies[i]; np != NULL; np = np->next)
			if (np->flag == BUDDY_FREE)
				goto balloc_found;

	/* If didnt find empty space */
	return NULL;

balloc_found:
	if (i == pow) {
		np->flag = BUDDY_ALLOCATED;
		return np + 1;
	}

	/* Dividing node */

	if (np == buddies[i]) {
		buddies[i] = np->next;
		buddies[i]->prev = NULL;
	}
	else {
		np->prev->next = np->next;
		np->next->prev = np->prev;
	}

	for (; i > pow; i--)
		addbuddy((void *)((int8_t *)np + (1 << (i - 1))), i - 1, BUDDY_FREE);

	addbuddy(np, i, BUDDY_ALLOCATED);

	return np + 1;
}

void
bfree(void *p)
{
	struct buddy_lst *np;
	int i;

	if (p == NULL)
		return;

	np = (struct buddy_lst *)p - 1;

	for (i = 0; i < MAXBUDDY; i++)
		for (np = buddies[i]; np != NULL; np = np->next)
			if (np == (struct buddy_lst *)p - 1)
				goto bfree_found;

	return;

bfree_found:
	if (np->prev && np->prev->flag == BUDDY_FREE && mergeable(np->prev, np, i)) {
		if (np->prev->prev) {
			np->prev->prev->next = np->next;
			np->next->prev = np->prev->prev;
		}
		else {
			buddies[i] = np->next;
			buddies[i]->prev = NULL;
		}

		addbuddy(np->prev, i + 1, BUDDY_FREE);
		return;
	}

	if (np->next && np->next->flag == BUDDY_FREE && mergeable(np, np->next, i)) {
		if (np->prev) {
			np->prev->next = np->next->next;
			np->next->next->prev = np->prev;
		}
		else {
			buddies[i] = np->next->next;
			buddies[i]->prev = NULL;
		}

		addbuddy(np, i + 1, BUDDY_FREE);
		return;
	}

	np->flag = BUDDY_FREE;
}

void
balloc_info()
{
	struct buddy_lst *node;
	int i;
	size_t free, used, total;

	free = used = 0;

	iprintf("\nBUDDY ALLOCATOR INFO:\n");

	for (i = 0; i <= MAXBUDDY; i++)
		for (node = buddies[i]; node != NULL; node = node->next) {
			iprintf("\tnode 0x%08x; flag = %d; size = 0x%08x; power = %d\n",\
					node, node->flag, 1 << i, i);
			if (node->flag)
				used += 1 << i;
			else
				free += 1 << i;
		}

	total = used + free;

	iprintf("\n\ttotal:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n", total >> 30,\
			(total >> 20) % 1024, (total >> 10) % 1024, total % 1024);
	iprintf("\tfree:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n", free >> 30,\
			(free >> 20) % 1024, (free >> 10) % 1024, free % 1024);
	iprintf("\tused:\t%d Gb\t%d Mb\t%d Kb\t%d bytes\n\n", used >> 30,\
			(used >> 20) % 1024, (used >> 10) % 1024, used % 1024);
}

void
balloc_init(int numpages)
{
	for (; numpages != 0; numpages--)
		addbuddy((void *)pgalloc(), 12, BUDDY_FREE);
}

