#ifndef _PHYSPGALLOC_H_
#define _PHYSPGALLOC_H_

#include "defs.h"
#include "mb_parce.h"


size_t physpgmalloc();
void physpgfree(size_t phys);

void physpginit(struct mm_area **mmap, int mmap_len);

#endif /* _PHYSPGALLOC_H_ */

