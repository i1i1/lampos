#include "buddyalloc.h"
#include "physpgalloc.h"
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
	struct buddy_lst *prev, *cur;
	int pow, i;

	/* Finding minimum power for this size */
	for (pow = 3; size > (1 << pow) - sizeof(struct buddy_lst); pow++)
		;

	/* Finding free area that fits */
	for (i = pow; i <= MAXBUDDY; i++) {
		if (buddies[i] == NULL)
			continue;

		if (buddies[i]->flag == 0) {
			prev = NULL;
			cur = buddies[i];
			buddies[i] = cur->next;
			goto found_free;
		}

		if (buddies[i]->next == NULL)
			continue;

		prev = buddies[i];
		cur = prev->next;

		for (; cur != NULL; prev = cur, cur = cur->next)
			if (cur->flag == 0)
				goto found_free;
	}

	/* If didnt found then return NULL */
	return NULL;

found_free:
	if (prev == NULL)
		buddies[i] = cur->next;
	else
		prev->next = cur->next;

	for (; i > pow; i--)
		addbuddy((void *)((uint8_t *)(cur) + (1 << (i - 1))), i - 1, 0);
	addbuddy(cur, i, 1);

	return prev + 1;
}

void
bfree(void *p)
{
	struct buddy_lst *ptr, *prev, *cur, *next;
	int i;

	if (p == NULL)
		return;

	ptr = (void *)((char *)p - sizeof(struct buddy_lst));

	/* Finding ptr in table of lists (result will be in 'next') */
	for (i = 0; i <= MAXBUDDY; i++) {
		if (buddies[i] == NULL)
			continue;

		if (buddies[i] == ptr) {
			prev = NULL;
			cur = NULL;
			next = buddies[i];
			goto found_node;
		}

		if (buddies[i]->next == NULL)
			continue;

		prev = NULL;
		cur = buddies[i];
		next = cur->next;

		for (; next != NULL; prev = cur, cur = next, next = next->next)
			if (next == ptr)
				goto found_node;
	}

	/* If didnt found then exit */
	return;

found_node:

	if (cur != NULL && cur->flag == 0 && mergeable(cur, next, i)) {
		if (prev == NULL)
			buddies[i] = next->next;
		else
			prev->next = next->next;
		addbuddy(cur, i + 1, 0);
		return;
	}

	if (next->next != NULL && next->next->flag == 0\
			&& mergeable(next, next->next, i)) {
		if (prev == NULL)
			buddies[i] = NULL;
		else
			cur->next = next->next->next;
		addbuddy(next, i + 1, 0);
		return;
	}

	next->flag = 0;
}

void
balloc_info()
{
	struct buddy_lst *node;
	int i;
	size_t free, used;

	free = used = 0;

	iprintf("\nBUDDY ALLOCATOR INFO:\n");

	for (i = 0; i <= MAXBUDDY; i++)
		for (node = buddies[i]; node != NULL; node = node->next) {
			iprintf("\tnode 0x%x; flag = %d; power = %d\n", node, node->flag, i);
			if (node->flag)
				used += 1 << i;
			else
				free += 1 << i;
		}

	iprintf("\n\ttotal:\t%d kilobytes\n", (free + used) / 1024);
	iprintf("\tfree:\t%d kilobytes\n", free / 1024);
	iprintf("\tused:\t%d kilobytes\n\n", used / 1024);
}

void
balloc_init()
{
	int i;

	for (i = 0; i < 0x100; i++)
		addbuddy((void *)physpgalloc(), 12, 0);
}

