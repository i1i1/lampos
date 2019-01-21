#include "sort.h"
#include "defs.h"


void
swap(void *a, void *b, size_t size)
{
	char ch;
	char *c, *d;

	c = a;
	d = b;

	while (size--) {
		ch = *c;
		*c++ = *d;
		*d++ = ch;
	}
}

void
sort(void *base, size_t nmeb, size_t size, \
			int (*cmp)(const void *, const void *))
{
	char *arr;
	size_t i, j, idx;

	arr = base;

	for (i = 0; i < nmeb - 1; i++) {
		idx = i;

		for (j = i + 1; j < nmeb; j++) {
			if (cmp(arr + idx * size, arr + j * size) == 1)
				idx = j;
		}

		if (idx != i)
			swap(arr + idx * size, arr + i * size, size);

	}
}

