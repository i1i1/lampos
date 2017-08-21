#include <stdint.h>

#include "sort.h"
#include "defs.h"


void
swap(void *a, void *b, size_t size)
{
	char c;

	while (size--) {
		c = *(char *)a;
		*(char *)a++ = *(char *)b;
		*(char *)b++ = c;

	}
}

void
sort(void *base, size_t nmeb, size_t size, int (*cmp)(const void *, const void *))
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

