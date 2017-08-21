#ifndef _SORT_H_
#define _SORT_H_

#include "defs.h"


void sort(void *base, size_t nmeb, size_t size,
			int (*cmp)(const void *, const void *));

#endif
