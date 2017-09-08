#include "buddyalloc.h"
#include "physpgalloc.h"
#include "kernel.h"


struct buddy_lst *buddies[MAXBUDDY + 1] = {NULL};


/* Function that finds buddy at 'addr' with flag = 'flag'
 * with minimum power = 'initpow'.
 * If 'addr' = 0 then all addreses match.
 * The result will be in '.next'.
 * If noone matches then '.next' == NULL.
 */
struct _findnode
findbuddy(struct buddy_lst *addr, int initpow, int flag)
{
	struct buddy_lst *prev, *cur, *next;
	int i;

	for (i = 0; i <= MAXBUDDY; i++) {
		/* If no elements in list */
		if (buddies[i] == NULL)
			continue;

		/* If buddies[i] matches */
		if (buddies[i]->flag == flag &&\
				(addr == NULL || buddies[i] == addr)) {
			prev = NULL;
			cur = NULL;
			next = buddies[i];
			goto found;
		}

		/* If 1 elements in list */
		if (buddies[i]->next == NULL)
			continue;

		prev = NULL;
		cur = buddies[i];
		next = cur->next;

		for (; next != NULL; prev = cur, cur = next, next = next->next)
			if (next->flag == flag && (addr == NULL || next == addr))
				goto found;
	}

	/* Didnt found */
	return (struct _findnode) { NULL, NULL, NULL, 0 };
found:
	return (struct _findnode) { prev, cur, next, i };
}

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
	struct buddy_lst *prev, *cur;

	if (addr == NULL)
		return;

	prev = buddies[power];

	if ((1 << power) <= sizeof(struct buddy_lst))
		return;

	/* If no elementes in list */
	if (prev == NULL) {
		buddies[power] = addr;
		buddies[power]->next = NULL;
		buddies[power]->flag = flag;

		return;
	}

	cur = prev->next;

	/* If only 1 element in list */
	if (cur == NULL) {
		if (flag == 0 && prev->flag == 0 && mergeable(prev, addr, power)) {
			buddies[power] = NULL;
			addbuddy(MIN(prev, addr), power + 1, 0);
			return;
		}

		if (prev < addr) {
			prev->next = addr;
			addr->next = NULL;
		}
		else {
			buddies[power] = addr;
			addr->next = prev;
		}

		addr->flag = flag;
		return;
	}

	/* Finding place for area
	 * (result will be between 'cur' and 'cur->next')
	 */
	for (; cur->next != NULL; prev = cur, cur = cur->next)
		if (cur->next > addr)
			break;

	/* Trying to merge with left one */
	if (flag == 0 && cur->flag == 0 && mergeable(cur, addr, power)) {
		prev->next = cur->next;
		addbuddy(MIN(cur, addr), power + 1, 0);
		return;
	}

	/* Trying to merge with right one */
	if (cur->next != NULL && flag == 0 && cur->next->flag == 0 &&\
					mergeable(addr, cur->next, power)) {
		cur->next = cur->next->next;
		addbuddy(addr, power + 1, 0);
		return;
	}

	/* Adding area to list */
	addr->next = cur->next;
	cur->next = addr;
	addr->flag = flag;
}

void *
balloc(size_t size)
{
	struct _findnode res;
	int pow;

	/* Finding minimum power for this size */
	for (pow = 3; size > (1 << pow) - sizeof(struct buddy_lst); pow++)
		;

	res = findbuddy(NULL, pow, 0);

	/* If didnt find empty space */
	if (res.next == NULL)
		return NULL;

	/* If list begins with found node */
	if (res.cur == NULL)
		buddies[res.power] = res.next->next;
	else
		res.cur->next = res.next->next;

	/* Dividing node */
	for (; res.power > pow; res.power--)
		addbuddy((void *)((uint8_t *)(res.next) + (1 << (res.power - 1))),\
					res.power - 1, 0);

	addbuddy(res.next, res.power, 1);

	return res.next + 1;
}

void
bfree(void *p)
{
	struct buddy_lst *ptr;
	struct _findnode res;

	if (p == NULL || p == (void *)(sizeof(struct buddy_lst)))
		return;

	ptr = (void *)((char *)p - sizeof(struct buddy_lst));

	res = findbuddy(ptr, 0, 1);

	/* If didnt find buddy marked by 'ptr' */
	if (res.next == NULL)
		return;

	/* If '.cur' and '.next' can be merged */
	if (res.cur != NULL && res.cur->flag == 0 && mergeable(res.cur, res.next, res.power)) {
		if (res.prev == NULL)
			buddies[res.power] = res.next->next;
		else
			res.prev->next = res.next->next;

		addbuddy(res.cur, res.power + 1, 0);
		return;
	}

	/* If '.next' and '.next->next' can be merged */
	if (res.next->next != NULL && res.next->next->flag == 0\
			&& mergeable(res.next, res.next->next, res.power)) {
		if (res.prev == NULL)
			buddies[res.power] = NULL;
		else
			res.cur->next = res.next->next->next;
		addbuddy(res.next, res.power + 1, 0);
		return;
	}

	res.next->flag = 0;
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
			iprintf("\tnode 0x%x; flag = %d; size = 0x%x; power = %d\n", node, node->flag, 1 << i, i);
			if (node->flag)
				used += 1 << i;
			else
				free += 1 << i;
		}

	total = used + free;

	iprintf("\n\ttotal:\t%d Gb %d Mb %d Kb %d bytes\n", total >> 30,\
		(total >> 20) % 1024, (total >> 10) % 1024, total % 1024);
	iprintf("\tfree:\t%d Gb %d Mb %d Kb %d bytes\n", free >> 30,\
			(free >> 20) % 1024, (free >> 10) % 1024, free % 1024);
	iprintf("\tused:\t%d Gb %d Mb %d Kb %d bytes\n", used >> 30,\
			(used >> 20) % 1024, (used >> 10) % 1024, used % 1024);
}

void
balloc_init(int numpages)
{
	for (; numpages != 0; numpages--)
		addbuddy((void *)physpgalloc(), 12, 0);
}

