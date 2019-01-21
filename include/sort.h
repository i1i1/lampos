#ifndef _SORT_H_
#define _SORT_H_

#include "defs.h"

typedef int cmp_t(const void *, const void *);


void sort(void *base, size_t nmeb, size_t size, cmp_t *cmp);

#endif /* _SORT_H_ */

